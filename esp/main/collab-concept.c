#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <string.h>

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

/** DEFINES **/
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define UDP_SUCCESS 1 << 0
#define UDP_FAILURE 1 << 1
#define MAX_FAILURES 10

// remote IP
#define INET_ADDR "192.168.0.71"
#define PORT_NUMBER 4050
#define TEAM_NAME "teamA"
#define AP_SSID "MY-AP-SSID"
#define AP_PASSWORD "PASSWORD"

/** GLOBALS **/

// event group to contain status information
static EventGroupHandle_t wifi_event_group;

// retry tracker
static int s_retry_num = 0;

// task tag
static const char *TAG = "WIFI";

/** FUNCTIONS **/

// event handler for wifi events
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    ESP_LOGI(TAG, "Connecting to AP...");
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < MAX_FAILURES) {
      ESP_LOGI(TAG, "Reconnecting to AP...");
      esp_wifi_connect();
      s_retry_num++;
    } else {
      xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
    }
  }
}

static void ip_event_handler(void *args, esp_event_base_t event_base,
                             int32_t event_id, void *event_data) {
  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
  }
}

esp_err_t connect_wifi() {
  int status = WIFI_FAILURE;

  ESP_ERROR_CHECK(esp_netif_init());

  // begin checking for events
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_event_group = xEventGroupCreate();

  esp_event_handler_instance_t wifi_handler_event_instance;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,
      &wifi_handler_event_instance));

  esp_event_handler_instance_t got_ip_event_instance;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL,
      &got_ip_event_instance));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = AP_SSID,
              .password = AP_PASSWORD,
              .threshold.authmode = WIFI_AUTH_WPA2_PSK,
              .pmf_cfg = {.capable = true, .required = false},
          },
  };

  // set the wifi controller to be a station
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  // set the wifi config
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  // start the wifi driver
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "STA intialization complete");

  // This right here blocks until either WIFI_SUCCESS or WIFI_FAILURE
  EventBits_t bits =
      xEventGroupWaitBits(wifi_event_group, WIFI_SUCCESS | WIFI_FAILURE,
                          pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & WIFI_SUCCESS) {
    ESP_LOGI(TAG, "Connected to ap");
    status = WIFI_SUCCESS;
  } else if (bits & WIFI_FAILURE) {
    ESP_LOGI(TAG, "Failed to connect to ap");
    status = WIFI_FAILURE;
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
    status = WIFI_FAILURE;
  }

  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
      IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
      WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
  vEventGroupDelete(wifi_event_group);

  return status;
}

// connect to the server and return the result
esp_err_t connect_tcp_server(void) {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    ESP_LOGE(TAG, "Failed to create a socket..?");
    return UDP_FAILURE;
  }

  struct sockaddr_in serverInfo = {0};
  serverInfo.sin_family = AF_INET;
  serverInfo.sin_addr.s_addr = inet_addr(INET_ADDR);
  serverInfo.sin_port = htons(PORT_NUMBER);
  if (inet_pton(AF_INET, INET_ADDR, &serverInfo.sin_addr) <= 0) {
    ESP_LOGE(TAG, "inet_pton failed");
    return UDP_FAILURE;
  }

  int msgLen = strlen(TEAM_NAME);
  if (sendto(sockfd, TEAM_NAME, msgLen, 0, (struct sockaddr *)&serverInfo,
             sizeof(struct sockaddr_in)) != msgLen) {

    ESP_LOGE(TAG, "sendto failed");
    return UDP_FAILURE;
  }

  return UDP_SUCCESS;
}

void app_main(void) {
  esp_err_t status = WIFI_FAILURE;
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // connect to wifi
  status = connect_wifi();
  if (WIFI_SUCCESS != status) {
    ESP_LOGI(TAG, "Failed to associate to AP, dying...");
    return;
  }

  status = connect_tcp_server();
  if (UDP_SUCCESS != status) {
    ESP_LOGI(TAG, "Failed to contact remote server, dying...");
    return;
  }
}
