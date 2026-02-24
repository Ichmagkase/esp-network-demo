import socket as s
import pygame
import sys

HOST = "0.0.0.0"
PORT = 4050
FPS = 1

PER_ROW = 3
BLOCK_SIZE = 200
BLOCK_MARGIN = 50
GLOBAL_PADDING = 25
TOTAL_SIZE = BLOCK_SIZE + BLOCK_MARGIN
SCREEN_W = PER_ROW * (BLOCK_SIZE + BLOCK_MARGIN)


def main():
    # get args
    args = sys.argv[1:]
    if len(args) < 1:
        print("Must list at least one team")
        print("usage: python server.py team1 team2")
        exit(1)

    # init socket
    print("listening on " + HOST + ":" + str(PORT))
    sock = s.socket(s.AF_INET, s.SOCK_DGRAM)
    sock.bind((HOST, PORT))
    sock.setblocking(0)

    # init pygame
    pygame.init()
    timer = pygame.time.Clock()
    screen_w = SCREEN_W
    screen_h = ((len(args) + PER_ROW - 1) // PER_ROW) * TOTAL_SIZE
    window = pygame.display.set_mode((screen_w, screen_h))

    font = pygame.font.SysFont(None, 48)

    RED = pygame.Color(255, 0, 0)
    GREEN = pygame.Color(0, 255, 0)
    WHITE = pygame.Color(255, 255, 255)

    teams = {}

    for i, team in enumerate(args):
        col = i % PER_ROW
        row = i // PER_ROW

        teams[team] = pygame.Rect(
            GLOBAL_PADDING + col * TOTAL_SIZE,
            GLOBAL_PADDING + row * TOTAL_SIZE,
            BLOCK_SIZE,
            BLOCK_SIZE,
        )
        pygame.draw.rect(window, RED, teams[team])

    pygame.display.update()

    while True:
        # Exit on quit
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                exit(0)

        # Draw team rectangles
        for team in teams:
            team_rect = teams[team]

            pygame.draw.rect(window, RED, team_rect)
            text_img = font.render(team, True, WHITE)
            text_rect = text_img.get_rect(center=team_rect.center)
            window.blit(text_img, text_rect)

        # Check data recieved
        try:
            data, addr = sock.recvfrom(1024)
            print("recieved message: ", data, " From: ", addr)
            decoded_data = data.decode("ascii").strip()

            pygame.draw.rect(window, GREEN, teams[decoded_data])
            text_img = font.render(decoded_data, True, WHITE)
            text_rect = text_img.get_rect(center=teams[decoded_data].center)
            window.blit(text_img, text_rect)

            del teams[decoded_data]

        except KeyError:
            print("Unknown team")
        except Exception as _:
            pass

        # switch on recieved data and update ui
        pygame.display.update()
        timer.tick(FPS)


if __name__ == "__main__":
    main()
