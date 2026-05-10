#!/usr/bin/env python3
import csv
import struct
import zlib
from collections import defaultdict


WIDTH = 1200
HEIGHT = 760
LEFT = 90
RIGHT = 260
TOP = 50
BOTTOM = 80

COLORS = [
    (230, 57, 70),
    (29, 53, 87),
    (69, 123, 157),
    (42, 157, 143),
    (233, 196, 106),
    (244, 162, 97),
    (131, 56, 236),
    (255, 0, 110),
    (0, 150, 199),
]


def set_pixel(image, x, y, color):
    if 0 <= x < WIDTH and 0 <= y < HEIGHT:
        image[y][x] = color


def draw_line(image, x0, y0, x1, y1, color):
    dx = abs(x1 - x0)
    dy = -abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    error = dx + dy

    while True:
        set_pixel(image, x0, y0, color)
        if x0 == x1 and y0 == y1:
            break
        doubled = 2 * error
        if doubled >= dy:
            error += dy
            x0 += sx
        if doubled <= dx:
            error += dx
            y0 += sy


def draw_rect(image, x0, y0, x1, y1, color):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            set_pixel(image, x, y, color)


DIGITS = {
    "0": ("111", "101", "101", "101", "111"),
    "1": ("010", "110", "010", "010", "111"),
    "2": ("111", "001", "111", "100", "111"),
    "3": ("111", "001", "111", "001", "111"),
    "4": ("101", "101", "111", "001", "001"),
    "5": ("111", "100", "111", "001", "111"),
    "6": ("111", "100", "111", "101", "111"),
    "7": ("111", "001", "010", "010", "010"),
    "8": ("111", "101", "111", "101", "111"),
    "9": ("111", "101", "111", "001", "111"),
    ".": ("000", "000", "000", "000", "010"),
    " ": ("000", "000", "000", "000", "000"),
}


def draw_digits(image, text, x, y, color, scale=2):
    cursor = x
    for ch in text:
        glyph = DIGITS.get(ch, DIGITS[" "])
        for row, line in enumerate(glyph):
            for col, bit in enumerate(line):
                if bit == "1":
                    draw_rect(
                        image,
                        cursor + col * scale,
                        y + row * scale,
                        cursor + (col + 1) * scale - 1,
                        y + (row + 1) * scale - 1,
                        color,
                    )
        cursor += 4 * scale


def write_png(path, image):
    raw = b"".join(b"\x00" + bytes(channel for pixel in row for channel in pixel) for row in image)

    def chunk(name, data):
        body = name + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", WIDTH, HEIGHT, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9))
    png += chunk(b"IEND", b"")

    with open(path, "wb") as output:
        output.write(png)


def main():
    series = defaultdict(lambda: {"x": [], "y": []})

    with open("10.05_collisions.csv", newline="", encoding="utf-8") as source:
        for row in csv.DictReader(source):
            name = row["function"]
            series[name]["x"].append(int(row["strings"]))
            series[name]["y"].append(int(row["collisions"]))

    max_x = max(max(values["x"]) for values in series.values())
    max_y = max(max(values["y"]) for values in series.values())
    max_y = max(max_y, 1)

    plot_width = WIDTH - LEFT - RIGHT
    plot_height = HEIGHT - TOP - BOTTOM

    image = [[(255, 255, 255) for _ in range(WIDTH)] for _ in range(HEIGHT)]

    axis = (40, 40, 40)
    grid = (225, 225, 225)
    for i in range(6):
        x = LEFT + i * plot_width // 5
        draw_line(image, x, TOP, x, TOP + plot_height, grid)
        draw_digits(image, str(i * max_x // 5), x - 25, TOP + plot_height + 18, axis, 2)

        y = TOP + plot_height - i * plot_height // 5
        draw_line(image, LEFT, y, LEFT + plot_width, y, grid)
        draw_digits(image, str(i * max_y // 5), 20, y - 5, axis, 2)

    draw_line(image, LEFT, TOP, LEFT, TOP + plot_height, axis)
    draw_line(image, LEFT, TOP + plot_height, LEFT + plot_width, TOP + plot_height, axis)

    for index, (name, values) in enumerate(series.items()):
        color = COLORS[index % len(COLORS)]
        points = []
        for x_value, y_value in zip(values["x"], values["y"]):
            x = LEFT + x_value * plot_width // max_x
            y = TOP + plot_height - y_value * plot_height // max_y
            points.append((x, y))

        for start, end in zip(points, points[1:]):
            draw_line(image, start[0], start[1], end[0], end[1], color)
        for x, y in points[:: max(1, len(points) // 16)]:
            draw_rect(image, x - 2, y - 2, x + 2, y + 2, color)

        legend_y = TOP + index * 28
        draw_rect(image, WIDTH - RIGHT + 20, legend_y, WIDTH - RIGHT + 45, legend_y + 12, color)
        draw_digits(image, str(index + 1), WIDTH - RIGHT + 55, legend_y, axis, 2)
        print(f"{index + 1}. {name}: {color}")

    write_png("10.05_collisions.png", image)


if __name__ == "__main__":
    main()
