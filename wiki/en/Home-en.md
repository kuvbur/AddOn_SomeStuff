> 🇷🇺 [Русский](#-somestuff--дополнение-для-archicad) | 🇬🇧 [English](#-somestuff--archicad-add-on)

# 🇬🇧 SomeStuff — Archicad Add-on

SomeStuff solves specialized automation tasks in Archicad: synchronizing properties with GDL parameters, element numbering, construction analysis, drawing layout automation, and more.

---

## 🚀 Getting Started

|                                                                                                             |                                                                           |
| ----------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| [Installation (WIN / MAC)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Install-en)                       | Download, copy to the Add-Ons folder, add commands to the menu            |
| [FAQ](https://github.com/kuvbur/AddOn_SomeStuff/wiki/FAQ-en)                                                | Why this add-on, what to do if nothing works, performance notes           |
| [Full Property Command Reference](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Property-Commands-List-en) | Complete reference: `Sync_from`, `Sync_to`, flags, formulas, MEP commands |
| [Testing / Beta Builds](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Testing-Help-en)                     | How to download builds from GitHub Actions and join testing               |

---

## 🔗 Property & GDL Synchronization

Two-way data exchange between Archicad properties and GDL parameters of library elements.

### [GDL Parameter ↔ Property Sync](https://github.com/kuvbur/AddOn_SomeStuff/wiki/GDLParameter2Property-en)

- Copy GDL parameter **into a property**: `Sync_from{PARAM_NAME}`
- Copy a property **into a GDL parameter**: `Sync_to{PARAM_NAME}`
- Copy one property into another: `Sync_from{Property:GROUP/PROPERTY}`
- Modes: **Monitor** (auto on change) / **Sync All** / **Sync Selected**

### [GUID-based and Hierarchical Sync](https://github.com/kuvbur/AddOn_SomeStuff/wiki/LinkPropertyByGUID-en)

- Curtain walls, zones and other hierarchical structures: `Sync_from_sub{...}` / `Sync_to_sub{...}`
- Link arbitrary elements via GUID: `Sync_from_GUID{...}`

---

## Constructions & Materials

### [Output Construction Layers to Properties](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Construction-Composition-en)

A template string in the property description generates a text composition of composite walls and complex profiles.

- Layer order: `Layers` / `Layers_inv` / `Layers_auto`
- Thickness, hatch, density, custom material properties
- Component volumes and areas (`%area%`, `%volume%`, `%qty%`)
- Formulas with per-layer summation
- Label formatting (`\CRLF`, `\TAB`, etc.)

### [Create Schedule Elements (Spec_rule)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Breakdown-Composites-Create-Elements-en)

Generates schedule rows for composite GDL objects (lintels, rebar, structural sections, materials).

- `Spec_rule` — basic version
- `Spec_rule_v2` — with auto-update and change highlighting
- `Spec_rule_v3` — silently skips elements with missing parameters
- Supports assemblies, rebar, rolled sections and materials from the kuvbur library

### [Finishing Schedule](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Finishing-en)

Analyses construction layers adjacent to a zone and collects finishing layer data. Can create finish elements (similar to Interior Elevation wizard) and/or write the composition into zone properties.

---

## Element Numbering

### [Element Numbering](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Element-Renumbering-en)

An ID Manager alternative that writes positions into properties.

- Numbering flag in the property description: `Renum_flag{property_name}`
- Auto-padding with zeros or spaces: `NULL`, `SPACE`, `ALLNULL`, `n_NULL`, etc.
- Grouping by criterion and partition properties
- Write position to Element ID: `Sync_to{ID}`

---

## Dimensions & Rounding

### [Dimension Processing](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Dimension-Functions-en)

Automatically checks and processes dimensions on the active view using rules defined in Project Info (`Addon_Dimensions`).

- Rules by layer name or dimension line pen
- Highlight non-multiple dimensions, replace text
- Hide wall thicknesses (`DeleteWall`), reset overridden values (`ResetText`)
- Classic rounding mode (`ClassicRound`)

### [Numeric Value Rounding in Properties](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Rounding-Thicknesses-Properties-en)

Rounding format is set in the property name after a dot: `1mm`, `01mm`, `01mp`, etc.

## Classification

### [Automatic Element Classification](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Automatic-Element-Classification-en)

- Auto-assign class to unclassified elements: add `some_stuff_class` to the class description
- Change class when a property changes: `Sync_to{Class:CLASSIFICATION_NAME}`
- Output full class name to a property: `Sync_from{Class:CLASSIFICATION_NAME; FullName}`

---

## Drawing Documentation

### [QR Code Output from Property](https://github.com/kuvbur/AddOn_SomeStuff/wiki/QR-Code-Output-en)

Generates a string for the `macro_qrcode` GDL macro from a property value: `Sync_from{QRCode:Property:PROPERTY_NAME}`. Supports output in 2D/3D scripts, labels and IFC.
