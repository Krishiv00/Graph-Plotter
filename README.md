# Graph-Plotter

A lightweight, interactive **2D graph plotting application** written in C++, designed for visualising mathematical equations in real time.

The application supports function graphs, inverse-style graphs, and parametric equations, with smooth panning, zooming, and live previews.

---

## Features

* Real-time 2D graph rendering
* Support for:

  * `y = f(x)` equations
  * `x = f(y)` equations
  * Parametric equations `x = g(t), y = f(t)`
* Live preview while typing expressions
* Multiple graphs rendered simultaneously
* Customisable domain per graph
* Smooth mouse-based navigation

---

## Controls

| Action                             | Input                               |
| ---------------------------------- | ----------------------------------- |
| Pan viewport                       | Hold **Left Mouse Button** and drag |
| Zoom in / out                      | **Mouse Scroll Wheel**              |
| Reset viewport                     | **C**                               |
| Remove all graphs                  | **R**                               |
| Start equation input               | **Input**                           |
| Toggle live preview (while typing) | **Ctrl + P**                        |
| Register equation                  | **Enter**                           |

---

## Entering Valid Equations

### 1. Function graphs (`y = f(x)`)

* Use **`x`** as the variable
* Do **not** use any other alphabet as a variable

Example:

```
x * x + 3
```

---

### 2. Inverse-style graphs (`x = f(y)`)

* Use **`y`** as the variable instead of `x`

Example:

```
sin(y) / cos(y)
```

---

### 3. Parametric equations

For parametric equations:

```
x = g(t)
y = f(t)
```

Use **`t`** as the variable and separate the two expressions with a comma:

```
f(t), g(t)
```

Example:

```
cos(t), sin(t)
```

---

## Domain Specification

By default, the domain of a graph is **-1 to 1**.

You can customise the domain by appending a range block after the equation:

```
{low < variable < high}
```

or using inclusive bounds:

```
{low <= variable <= high}
```

### Expression-Based Domains

The `low` and `high` values are **not limited to numeric literals**.
They are parsed as full mathematical expressions and evaluated before plotting.

This allows domains to be defined using constants and expressions.

Example:

```
x * x { -2 * pi <= x <= 2 * pi }
```

---

## Example Graphs

Try the following equations to explore the capabilities of the plotter.

---

### ❤️ Heart Curve (Parametric)

A classic parametric heart shape.

```
(0.05 * 16 * sin(t) * sin(t) * sin(t), 0.05 * (13 * cos(t) - 5 * cos(2 * t) - 2 * cos(3 * t) - cos(4 * t))) { -pi <= t <= pi }
```

---

### 🕸 Rose Curve

A clean polar-style curve with multiple petals.

```
(cos(4 * t) * cos(t), cos(4 * t) * sin(t)) { 0 <= t <= 2*pi }
```

---

### 🌀 Lissajous Curve

A classic Lissajous figure created from sinusoidal components with different frequencies.

```
(sin(2 * t), cos(3 * t)) { -5 <= t <= 5 }
```

---

### 🌊 Highly Oscillatory Wave

A function with increasing oscillation near the origin.

```
x * sin(1 / x) { -pi <= x <= pi }
```

---

### ⚡ Chaotic Rational Function

A highly non-linear function that produces extreme oscillations.

```
15 * ((sin(x) ^ 3 * 15) - 15) / ((sin(x) ^ 2 * 15) ^ - 1 - 15) { -100 <= x <= 100 }
```

---

### ∞ Lemniscate of Bernoulli

A classic figure-eight curve defined parametrically.

```
(cos(t) / (1 + sin(t) ^ 2), sin(t) * cos(t) / (1 + sin(t) ^ 2)) { -pi <= t <= pi }
```

---

### 🧩 Butterfly Curve

Complex but bounded parametric curve.

```
(sin(t) * (exp(cos(t)) - 2 * cos(4 * t) - sin(t / 12) ^ 5), cos(t) * (exp(cos(t)) - 2 * cos(4 * t) - sin(t / 12) ^ 5)) { 0 <= t <= 12 * pi }
```

---

## Dependencies

* **SFML** — rendering and window management
* **tinyexpr** — mathematical expression parsing

All third-party dependencies are lightweight and cross-platform.

---

## Notes

* Variables must strictly match the equation type (`x`, `y`, or `t`)
* Invalid expressions are safely rejected