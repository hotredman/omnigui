# OmniKit: API rules

OmniKit is built on a handful of rules that apply to every component. If you know them, you can guess the signature of any widget.

> The demo ([OmniKitShowcase.cpp](../examples/demo/OmniKitShowcase.cpp)) never calls `ImGui::` directly. That is the test of API purity: everything the application needs is in OmniKit.

## 1. Three kinds of components

| Kind | Form | Examples |
|---|---|---|
| **Widget**: an immediate function | `Widget(data, {options})` | `Button`, `InputField`, `Combo`, `Toggle`, `SearchInput`, `ToolButton`, `Badge`, `Tag`, `TabBar`, `PresetGrid`, `ValueDisplay`, `StatusBanner` |
| **Scope**: an RAII area | `if (Scope s({options}); s) { ... }` | `Card`, `Header`, `Toolbar`, `Sidebar`, `SidePanel`, `Modal`, `List`, `TableGrid`, `ColumnLayout`, `GridLayout`, `FilterBar`, `ItemRow`, `ActionGroup`, `IdScope` |
| **Stateful object**: lives between frames | a class member, `Render` / `Draw` per frame | `Indicator`, `EditableLabel`, charts, `Table<T>`, `ConfirmModal` |

### Scope

* The constructor opens the area and the destructor always closes it. There are no public `Begin()`/`End()`.
* `explicit operator bool` means "the area is open and you can draw in it".
* A scope cannot be copied or moved. Create it on the stack and use it within one frame.
* Child areas are scopes too: `if (auto left = bar.Left()) { ... }`.
* `Card` has its own 12-column grid (`card.Col(...)`, `.col = Col::Half()` in field options) because its rows take their height from `UiSize`. Free-form layouts use `ColumnLayout`.

```cpp
if (Card card({.title = "Network"}); card) {
    card.Text(address, {.label = "Address", .col = Col::TwoThirds()});
    card.Int(port,     {.label = "Port",    .col = Col::Third()});
    if (card.Button({.label = "Apply", .variant = UiVariant::Primary})) { ... }
}
```

## 2. Options structs

* All appearance goes into an options struct with C++20 designated initializers. Fields are written in declaration order.
* **Positional arguments are only for data** (`value`, `items`, `bool&`) **or display-only content** (`Text`, `Badge`, `Tag`, the title of a `List` header).
* A caption is always called `label`; a second line is called `sublabel`.

```cpp
Button({.label = "Save", .variant = UiVariant::Primary, .icon = Icon::Check});
InputField(rate, {.label = "Rate", .unit = "Hz", .width = Fill});
Badge("Online", {.variant = UiVariant::Success});
```

## 3. Identity (ImGui ID)

* `key` is an optional explicit identity. If it is missing, `label` is used, and then the address of the data.
* Widgets repeated in a loop must be told apart: use `key = ...` or wrap the iteration in `IdScope`.
* A stateful object takes its identity from `this` (or from `key` in its options).

```cpp
for (int i = 0; i < runs.size(); ++i) {
    IdScope id(i);
    if (ActionGroup actions; actions) {
        if (actions.ToolButton({.icon = Icon::Trash, .tooltip = "Delete"})) { ... }
    }
}
```

## 4. Sizes

* `width` / `height` are in **base pixels**: the library scales them by the theme scale (`UiTheme::SetScale`).
* `0` means automatic (by content or default). The constant `Fill = -1.0f` means "to the edge of the container".
* Fields with the `Px` suffix (`widthPx`, `heightPx`, `posYPx`, `sizePx`) are **final pixels**. They exist for containers and shell geometry only (`Header`, `Toolbar`, `Sidebar`, `ContentArea`, `ColumnLayout`).

## 5. `UiSize`: the size of controls

`UiSize` (`Mini`, `Small`, `Medium`, `Large`) sets the height and the font of every **control and small element**:

`Button`, `InputField`, `Combo`, `SearchInput`, `ToolButton`, `Toggle`, `Tag`, `Badge`, `TabBar`, `PresetGrid`, `ValueDisplay`, and field labels.

* The height is exactly `UiTheme::GetMetrics(size).height`.
* Chips (`Badge`, `Tag`) scale their font, padding and radius by `UiTheme::SizeFactor(size)`.
* **Structural containers** (`Card`, `Header`, `Toolbar`, `Sidebar`, `SidePanel`, `Modal`, `Panel`, `TableGrid`) do not use `UiSize`. They take their look from theme styles, and their height from `height` in base pixels.

## 6. Theme

* All colors, fonts and spacing come from `UiTheme::Get()` (`palette`, `card`, `table`, ...).
* A component may take a style override through a `const XStyle* style = nullptr` field in its options. `nullptr` means the style from the theme.
* The application sets the scale itself: `UiTheme::Get().SetScale(dpiScale)`. The library does not query the OS.

## 7. Platform independence

The library depends only on the standard library, ImGui and the renderer. What touches the OS lives in `src/core`: `Assets` (SDL path), `PathUtil` (UTF-8 paths), `TimeUtil` (local time), `ProcessStats` (process CPU and memory).
