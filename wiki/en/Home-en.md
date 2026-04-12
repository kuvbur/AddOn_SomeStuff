# SomeStuff — Add-on for Archicad

The add-on solves specialized automation tasks in Archicad: synchronization of properties and GDL parameters, numbering, structural analysis, drawing documentation, and much more.

---

## Getting Started

|                                                                                                        |                                                                                |
| ------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------ |
| [Installation (WIN / MAC)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Install-en)                     | Download, copy to the add-ons folder, add commands to the menu               |
| [FAQ](https://github.com/kuvbur/AddOn_SomeStuff/wiki/FAQ-en)                                           | Why this add-on, what to do if nothing works, impact on performance |
| [List of All Property Commands](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Property-Commands-List-en) | Complete reference: `Sync_from`, `Sync_to`, flags, formulas, MEP commands         |
| [Test Versions](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Testing-Help-en)                      | How to download a build from Actions and join testing                  |
| [Condensed Help for LLM](https://github.com/kuvbur/AddOn_SomeStuff/wiki/SomeStuff-AI-Context-en.md)    | Use to connect to AI for answering questions                         |

---

## 🔗 Property and GDL Synchronization

Bidirectional data exchange between Archicad properties and GDL parameters of library elements.

### [Synchronizing GDL Parameters and Properties](https://github.com/kuvbur/AddOn_SomeStuff/wiki/GDLParameter2Property-ru)

- Copy a GDL parameter **to a property**: `Sync_from{PARAMETER_NAME}`
- Copy a property **to a GDL parameter**: `Sync_to{PARAMETER_NAME}`
- Copy one property to another: `Sync_from{Property:GROUP/PROPERTY}`
- Modes: **Track** (auto on change) / **Synchronize All** / **Synchronize Selected**

### [Synchronization by GUID and Nested Elements](https://github.com/kuvbur/AddOn_SomeStuff/wiki/LinkPropertyByGUID-ru)

- Curtain Walls, Zones, and other hierarchical structures: `Sync_from_sub{...}` / `Sync_to_sub{...}`
- Linking arbitrary elements via GUID: `Sync_from_GUID{...}`

---

## Structures and Materials

### [Construction Composition Output in Properties](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Construction-Composition-ru)

A template string in the property description generates the textual composition of multi-layer constructions and complex profiles.

- Layer order: `Layers` / `Layers_inv` / `Layers_auto`
- Thickness, fill, density, custom material properties
- Component volumes and areas (`%area%`, `%volume%`, `%qty%`)
- Formulas with summation across layers
- Formatting for labels (`\CRLF`, `\TAB`, etc.)

### [Create Specification Elements (Spec_rule)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Breakdown-Composites-Create-Elements-ru)

Generates specification lines for composite GDL objects (lintels, reinforcement, rolled sections, materials).

- `Spec_rule` — base version
- `Spec_rule_v2` — with auto-update and change highlighting
- `Spec_rule_v3` — with ignoring elements lacking required parameters
- Support for assemblies, reinforcement, rolled sections, and materials from the kuvbur library

### [Finishing Schedule](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Finishing-ru)

Analyzes constructions adjacent to a zone, collects data about finishing layers. Can create finishing elements (analogous to Interior Designer) and/or record the composition into zone properties.

---

## Element Numbering

### [Element Renumbering](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Element-Renumbering-ru)

Analog of the ID Manager with position preservation in properties.

- Renumbering flag in the property description: `Renum_flag{property_name}`
- Auto-fill with zeros / spaces: `NULL`, `SPACE`, `ALLNULL`, `n_NULL`, etc.
- Grouping by criteria and splitting
- Writing the position to the element's ID: `Sync_to{ID}`

---

## Dimensions and Rounding

### [Working with Dimensions](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Dimension-Functions-ru)

Automatic checking and processing of dimensions on the active view according to rules from the project information (`Addon_Dimensions`).

- Rules by dimension layer or pen
- Highlighting non-multiple dimensions, text replacement
- Hiding wall thicknesses (`DeleteWall`), resetting overridden values (`ResetText`)
- Classic rounding (`ClassicRound`)

### [Rounding Numerical Values in Properties](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Rounding-Thicknesses-Properties-ru)

The rounding format is specified in the property name after a dot: `1mm`, `01mm`, `01mp`, etc.

---

## 🗺️ Profiles and Drawings

### [Build Profile by Line](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Build-Profile-Line-ru)

Creates 3D section documents along a morph-line or fence for constructing utility network profiles according to GOST.

1. Worksheet with a hotspot (pen 163) → general layout
2. Morph/fence with ID `SITE_NAME@SCALE`
3. Run **Build Profile by Line** → 3D documents are created
4. Place on layout → **Align Selected Drawings**

---

## Classification

### [Automatic Element Classification](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Automatic-Element-Classification-ru)

- Automatic class assignment for unclassified elements: add `some_stuff_class` to the class description
- Changing class when a property is modified: `Sync_to{Class:CLASSIFICATION_NAME}`
- Outputting the full class name to a property: `Sync_from{Class:CLASSIFICATION_NAME; FullName}`

---

## Documentation Formatting

### [Layout Changes per GOST 21.1101](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Layout-Changes-ru)

Populates stamps and revision markers on layouts in accordance with GOST R 21.101-2020. Compatible with the [kuvbur_Format per GOST](https://github.com/kuvbur/gdl_bibl) library.

### [QR Code Output from Property](https://github.com/kuvbur/AddOn_SomeStuff/wiki/QR-Code-Output-ru)

Generates a string for the `macro_qrcode` macro based on a property value: `Sync_from{QRCode:Property:PROPERTY_NAME}`. Supports output in 2D and 3D scripts, labels, and IFC.

---