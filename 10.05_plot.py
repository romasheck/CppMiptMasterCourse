#!/usr/bin/env python3
import csv
from collections import OrderedDict
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


WIDTH = 1500
HEIGHT = 820
LEFT = 110
RIGHT = 520
TOP = 70
BOTTOM = 105

CSV_PATH = Path("10.05_collisions.csv")
PNG_PATH = Path("10.05_collisions.png")

COLORS = [
    (214, 39, 40),
    (31, 119, 180),
    (44, 160, 44),
    (148, 103, 189),
    (255, 127, 14),
    (23, 190, 207),
    (140, 86, 75),
    (227, 119, 194),
    (127, 127, 127),
]


def load_font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    candidates = [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" if bold else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf" if bold else
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
    ]

    for candidate in candidates:
        path = Path(candidate)
        if path.exists():
            return ImageFont.truetype(str(path), size)

    return ImageFont.load_default()


def read_series() -> OrderedDict[str, dict[str, list[int]]]:
    series: OrderedDict[str, dict[str, list[int]]] = OrderedDict()

    with CSV_PATH.open(newline="", encoding="utf-8") as source:
        for row in csv.DictReader(source):
            name = row["function"]
            series.setdefault(name, {"x": [], "y": []})
            series[name]["x"].append(int(row["strings"]))
            series[name]["y"].append(int(row["collisions"]))

    return series


def nice_number(value: int) -> str:
    if value >= 1_000_000:
        return f"{value / 1_000_000:.1f}M".rstrip("0").rstrip(".")
    if value >= 1_000:
        return f"{value // 1_000}K"
    return str(value)


def draw_text_centered(draw: ImageDraw.ImageDraw, xy: tuple[int, int], text: str,
                       font: ImageFont.ImageFont, fill: tuple[int, int, int]) -> None:
    bbox = draw.textbbox((0, 0), text, font=font)
    width = bbox[2] - bbox[0]
    height = bbox[3] - bbox[1]
    draw.text((xy[0] - width // 2, xy[1] - height // 2), text, font=font, fill=fill)


def draw_vertical_label(image: Image.Image, text: str, center: tuple[int, int],
                        font: ImageFont.ImageFont, fill: tuple[int, int, int]) -> None:
    bbox = ImageDraw.Draw(image).textbbox((0, 0), text, font=font)
    text_image = Image.new("RGBA", (bbox[2] - bbox[0] + 8, bbox[3] - bbox[1] + 8), (255, 255, 255, 0))
    text_draw = ImageDraw.Draw(text_image)
    text_draw.text((4, 4), text, font=font, fill=fill)
    rotated = text_image.rotate(90, expand=True)
    image.alpha_composite(rotated, (center[0] - rotated.width // 2, center[1] - rotated.height // 2))


def main() -> None:
    series = read_series()
    if not series:
        raise RuntimeError("no data in 10.05_collisions.csv")

    max_x = max(max(values["x"]) for values in series.values())
    max_y = max(max(values["y"]) for values in series.values())
    max_y = max(max_y, 1)

    plot_width = WIDTH - LEFT - RIGHT
    plot_height = HEIGHT - TOP - BOTTOM
    plot_right = LEFT + plot_width
    plot_bottom = TOP + plot_height

    image = Image.new("RGBA", (WIDTH, HEIGHT), (255, 255, 255, 255))
    draw = ImageDraw.Draw(image)

    title_font = load_font(26, bold=True)
    label_font = load_font(18, bold=True)
    tick_font = load_font(14)
    legend_font = load_font(15)
    endpoint_font = load_font(13, bold=True)

    axis = (35, 35, 35)
    grid = (224, 224, 224)

    draw_text_centered(draw, ((LEFT + plot_right) // 2, 34), "Hash collisions for random unique lowercase strings",
                       title_font, axis)

    for i in range(6):
        x_value = i * max_x // 5
        x = LEFT + x_value * plot_width // max_x
        draw.line((x, TOP, x, plot_bottom), fill=grid, width=1)
        draw_text_centered(draw, (x, plot_bottom + 22), nice_number(x_value), tick_font, axis)

        y_value = i * max_y // 5
        y = plot_bottom - y_value * plot_height // max_y
        draw.line((LEFT, y, plot_right, y), fill=grid, width=1)
        draw.text((28, y - 8), str(y_value), font=tick_font, fill=axis)

    draw.line((LEFT, TOP, LEFT, plot_bottom), fill=axis, width=2)
    draw.line((LEFT, plot_bottom, plot_right, plot_bottom), fill=axis, width=2)

    draw_text_centered(draw, ((LEFT + plot_right) // 2, HEIGHT - 36), "Number of strings", label_font, axis)
    draw_vertical_label(image, "Collisions", (34, (TOP + plot_bottom) // 2), label_font, axis)

    endpoint_labels: list[tuple[int, str, tuple[int, int, int], int]] = []
    for index, (name, values) in enumerate(series.items()):
        color = COLORS[index % len(COLORS)]
        points = [
            (
                LEFT + x_value * plot_width // max_x,
                plot_bottom - y_value * plot_height // max_y,
            )
            for x_value, y_value in zip(values["x"], values["y"])
        ]

        draw.line(points, fill=color, width=3, joint="curve")
        for point in points[:: max(1, len(points) // 18)]:
            draw.ellipse((point[0] - 3, point[1] - 3, point[0] + 3, point[1] + 3), fill=color)

        endpoint_labels.append((points[-1][1], name, color, values["y"][-1]))

    endpoint_labels.sort()
    adjusted_labels = []
    last_y = TOP - 22
    for y, name, color, collisions in endpoint_labels:
        label_y = max(y, last_y + 19)
        adjusted_labels.append([label_y, y, name, color, collisions])
        last_y = label_y

    if adjusted_labels and adjusted_labels[-1][0] > plot_bottom - 16:
        shift = adjusted_labels[-1][0] - (plot_bottom - 16)
        for label in adjusted_labels:
            label[0] -= shift

    if adjusted_labels and adjusted_labels[0][0] < TOP + 4:
        shift = TOP + 4 - adjusted_labels[0][0]
        for label in adjusted_labels:
            label[0] += shift

    for label_y, y, name, color, collisions in adjusted_labels:
        label_y = int(label_y)

        draw.line((plot_right - 8, y, plot_right + 10, label_y + 7), fill=color, width=1)
        draw.text((plot_right + 14, label_y), f"{name} ({collisions})", font=endpoint_font, fill=color)

    legend_x = plot_right + 260
    draw.text((legend_x, TOP - 6), "Legend", font=label_font, fill=axis)
    draw.text((legend_x, TOP + 20), "final collision count", font=tick_font, fill=axis)

    for index, (name, values) in enumerate(series.items()):
        color = COLORS[index % len(COLORS)]
        legend_y = TOP + 56 + index * 34
        draw.line((legend_x, legend_y + 9, legend_x + 32, legend_y + 9), fill=color, width=4)
        draw.text((legend_x + 44, legend_y), f"{name}: {values['y'][-1]}",
                  font=legend_font, fill=axis)

    image.convert("RGB").save(PNG_PATH)
    print(f"wrote {PNG_PATH}")


if __name__ == "__main__":
    main()
