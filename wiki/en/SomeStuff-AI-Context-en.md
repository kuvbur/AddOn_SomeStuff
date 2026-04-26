# SomeStuff — AI Reference for Archicad Add-on

> File for integration into AI assistant context. Contains a concise reference for all functions of the SomeStuff add-on for Archicad. Terminology aligns with the official Archicad documentation and the community.graphisoft.com forum.

---

## What is SomeStuff

An Add-On for Archicad that automates work with Element Properties: synchronization of GDL parameters of Library Elements and Properties, numbering, analysis of Construction composition, drawing documentation according to GOST standards, management of Element Classification, and much more.

---

## Installation

**Windows:** Download from Releases (suffix WIN), close Archicad, copy to the extensions folder (`C:\Program Files\GRAPHISOFT\ARCHICAD XX\ARCHICAD Extensions\Add-Ons`), launch Archicad.

**Mac:** Download (suffix MAC), in Terminal execute:

```bash
xattr -cr SomeStuff.bundle
codesign --force --deep --sign - SomeStuff.bundle
```

Then in the Add-On Manager, click "Add" and specify the path to the file.

**Adding Commands to the Menu:** Options → Work Environment → Menu / Command Palette → "All Commands by Topic" → Extensions → SomeStuff → add the required commands.

---

## Key Operating Principle

All configuration is performed through the **"Description"** field of a Property in the Property Manager (Options > Property Manager). The add-on reads this field and executes the corresponding action.

### Prerequisites for Operation

1. The Element must be **classified** — The Class is assigned on the "Classifications and Properties" panel in the Element Parameters Dialog.
2. The Element must have a **Flag Property** — A property of type **Match Criteria** with the text `Sync_flag` in the "Description" field, set to a value of **TRUE**.
3. Properties must be **available** for the Element's Class — configured in the lower part of the Property Manager.
4. The required Element type must be **enabled** in the add-on's menu.

> **Diagnostics:** Navigator Panel → Report — the add-on outputs detailed information about each performed operation there.

---

## Processing Control Flags

Description is written in the "Description" field of the Property in the Property Manager.

| Property Description | Property Data Type | Purpose |
| -------------------- | ------------------ | ------- |
| `Sync_flag`          | Match Criteria     | Enable/disable all processing for the Element |
| `Sync_class_flag`    | Match Criteria     | Disable only Classification processing |
| `Sync_correct_flag`  | Match Criteria     | Disable only coordinate verification |
| `Sync_reset`         | any                | Reset the Property value to its default value everywhere |

---

## Add-on Control Commands

- **Monitor** — automatic synchronization on every element change
- **Synchronize All** — process all elements of the selected type in the project
- **Synchronize Selected** — process only the selected elements (without activating monitoring)

---

## Synchronization of Properties and GDL Parameters

The command description is written in the "Description" field of the **target** Property in the Property Manager.

### Core Commands

| Property Description                      | Action                                                             |
| ----------------------------------------- | ------------------------------------------------------------------ |
| `Sync_from{PARAMETER_NAME}`               | Copy GDL Parameter of Library Element → to Property                |
| `Sync_from{description:DISPLAY_NAME}`     | By display name of GDL Parameter (supports Cyrillic)               |
| `Sync_from{Property:GROUP/PROPERTY}`      | Copy value from another Property → to this Property                |
| `Sync_from{IFC:PROPERTY_NAME}`            | Copy value from IFC property → to Archicad Property                |
| `Sync_to{PARAMETER_NAME}`                 | Write Property value → to GDL Parameter of Library Element         |
| `Sync_to{id}`                             | Write Property value → to Element ID                               |

To find the GDL parameter name (Latin variable), use the Object Editor: File → Libraries and Objects → Open Object, click "Parameters" button → "Variable" column. For password-protected objects, use the "Dump Library Elements" command — it outputs the parameter list to the Report.

### Ignored Value Modifiers (added via `;`)

| Modifier     | Meaning                                                                 |
| ------------ | ----------------------------------------------------------------------- |
| `empty`      | Ignore empty strings and zero numbers                                   |
| `trim_empty` | Ignore whitespace-only strings and zero numbers                         |
| `def`        | Reset to default value if a suitable value is not found                 |

Example: `Sync_from{Property:Group/Property; empty}`

### Working with GDL Parameter Arrays (Read-Only)

| Modifier | Result for Text       | Result for Numbers |
| ----------- | -------------------------- | ------------------- |
| `uniq`      | Unique Values        | Sum               |
| `sum`       | Concatenation of all values | Sum               |
| `max`       | —                          | Maximum            |
| `min`       | —                          | Minimum             |

Example with a string range: `Sync_from{param_name; uniq(1,3)}`  
GDL parameters can be used as range boundaries: `Sync_from{param_name; uniq(1,nrow)}`

### Element Coordinates

| Property Description                                      | Data                                                                 |
| --------------------------------------------------------- | ------------------------------------------------------------------- |
| `Sync_from{symb_pos_x}` / `{symb_pos_y}` / `{symb_pos_z}` | Base coordinates (for Walls/Beams — start point)                    |
| `Sync_from{Coord:symb_pos_sx}` / `{symb_pos_sy}`          | Start coordinates of Wall/Beam                                       |
| `Sync_from{Coord:symb_pos_ex}` / `{symb_pos_ey}`          | End coordinates of Wall/Beam                                        |
| `Sync_from{Coord:symb_pos_lo_x}` etc.                    | Coordinates relative to user-defined origin                          |
| `Sync_from{Coord:symb_pos_sp_x}` etc.                    | Coordinates relative to Project Location Point (Survey Point)       |

Supported types: Window, Door, Wall, Beam, Column, Object, Zone. For Curtain Wall System panels returns the panel center (x, y, z).

### Hierarchical Structures (Curtain Wall System, Zones)

| Property Description                      | Action                                                                                         |
| ----------------------------------------- | ------------------------------------------------------------------------------------------------ |
| `Sync_from_sub{Property:GROUP/PROPERTY}` | Read Property from nested sub-element (Panel, Frame, Accessory of Curtain Wall System) |
| `Sync_to_sub{Property:GROUP/PROPERTY}`   | Write Property value to nested sub-element                                               |

### Linking Arbitrary Elements by GUID

1. Select the child elements (must be editable)
2. Run the "Link Elements" command
3. Click on the parent element (data source)

In the "Description" field of the child element's Property: `Sync_from_GUID{Property:GROUP/PROPERTY_C_GUID; Property:GROUP/PROPERTY}`

In the "Description" field of the property storing the parent's GUID: `Sync_GUID`

---

## Element Classification

Classes are managed by the Classification Manager (Options > Classification Manager). A class is assigned to an element in the "Classifications and Properties" panel of the Element Settings Dialog.

| Configuration Location                                      | Text in the "Description" field               | Action                                                                                   |
| ----------------------------------------------------------- | --------------------------------------------- | ---------------------------------------------------------------------------------------- |
| "Description" field of a **Class** in the Classification Manager | `some_stuff_class`                            | Automatically assign this Class to unclassified elements                                 |
| "Description" field of a **Property** (type: String)        | `Sync_to{Class:CLASSIFICATION_NAME}`          | Change the element's Class when the Property value changes (Class is searched by its ID) |
| "Description" field of a **Property** (type: String)        | `Sync_from{Class:CLASSIFICATION_NAME; FullName}` | Output the full name of the element's Class into the Property                            |

---

## Composition of Structures

The command description is written in the "Description" field of the target Property. Property names in the template string should be enclosed with `%` symbols.

### Basic Syntax

```
Sync_from{Material:Layers; "%Description% - %Thickness.2mm%mm. "}
```

| Keyword Variant              | Layer Order                                                       |
| ---------------------------- | ----------------------------------------------------------------- |
| `Material:Layers`            | As in the Composite Structure Editor (first layer from top)       |
| `Material:Layers_inv`        | In reverse order                                                  |
| `Material:Layers_auto`       | As in the Cross-Section 2D Preview                                |
| `Material:Layers,PEN_NUMBER` | Complex Profile layers defined by the specified Section Pen       |
| `Material:Layers,all`        | All layers with volume and area calculation                       |
| `Material:Layers,unic`       | Merge layers with identical Building Material and thickness       |

### Built-in Template String Variables

| Variable                                 | Value                                                     |
| ------------------------------------------ | ------------------------------------------------------------ |
| `%Толщина%` / `%layer_thickness%` / `%th%` | Layer Thickness of the Building Material                     |
| `%n%`                                      | Sequential layer number (all layers)                         |
| `%ns%`                                     | Sequential layer number (only layers with a non-empty string)|
| `%штриховка%`                              | Name of the Building Material Fill                           |
| `%описание%`                               | Description of the Building Material                         |
| `%наименование%`                           | Name of the Building Material                                |
| `%плотность%`                              | Density of the Building Material                             |
| `%area%`                                   | Component area, sq.m.                                        |
| `%volume%`                                 | Component volume, cu.m.                                      |
| `%qty%`                                    | Quantity in the selected unit of measurement                 |
| `%unit%`                                   | Unit of measurement (from the material property `some_stuff_units`) |
| `%bmat_inx%`                               | Building Material Index                                      |

### Output Formatting

- Special Characters for Labels: `\CRLF`, `\LF`, `\TAB`, `\PS`, `\LS`, `\NEL`
- Alignment in Interactive Catalogs: add `~200` (pad with spaces) or `@200` (with tab) before the closing quote in the rule
- Layer-based Formulas: the part of the expression within `< >` is repeated for each layer. Example: for a Wall 20+380+20 mm, the entry `<+2*%thickness.1mm%>` will yield `+2*20+2*380+2*20 = 840`

### Building Material Properties for Quantity Calculation

Properties are created and made available for Building Material Classes via Parameters > Element Properties > Building Materials.

| Building Material Property Description | Purpose                                                                           |
| -------------------------------------- | --------------------------------------------------------------------------------- |
| `some_stuff_units`                     | Unit of measurement: `sq.m.`, `m²`, `cub.m.`, `m³`, `lin.m.`, `not specified`     |
| `some_stuff_th`                        | Thickness of material with constant thickness (e.g., waterproofing). Data type: Length |
| `some_stuff_kzap`                      | Reserve coefficient (affects only the `%qty%` variable)                           |
| `Sync_name`                            | Set arbitrary text instead of the Building Material name in the composition       |

For a custom Property to be read from the Building Material, not from the Archicad element, add to its "Description" field: `{@property:buildingmaterialproperties}`

---

## Rounding of Property Numerical Values

The format is specified in the **name** of the Property after a dot — this part must contain the letter `m`:

| Property Name Format | Example Result                                          |
| -------------------- | ------------------------------------------------------- |
| `MyProperty.1mm`     | Millimeters, 1 decimal, trailing zeros suppressed: `12` or `12.1` |
| `MyProperty.01mm`    | Millimeters, 1 decimal, trailing zeros kept: `12.0` or `12.1` |
| `MyProperty.01mp`    | Millimeters, decimal separator is a dot: `12.0` or `12.1` |

In a Building Material composition template string: `%Thickness.2mm%`  
In a Property description during synchronization: `Sync_from{A.1mm}`

---

## Element Numbering (ID Manager Analog)

Two Properties are required in the Property Manager: a **flag property** and a **position property**. The numbering rules are written in the "Description" field of each.

### Renumbering Flag Property

Data Type: **Match Criteria** or **Parameter Set** (values: Add / Renumber / Exclude).

| Flag Property Description                          | Behavior                                                      |
| ------------------------------------------------ | -------------------------------------------------------------- |
| `Renum_flag{Property:Group/PropertyWithRule}` | Standard renumbering                                          |
| `Renum_flag{...; NULL}`                          | Zero-padding, length determined by the maximum position in the group    |
| `Renum_flag{...; SPACE}`                         | Space-padding, length determined by the maximum position in the group |
| `Renum_flag{...; ALLNULL}`                       | Zero-padding without considering groups                              |
| `Renum_flag{...; 4_NULL}`                        | Fixed length of 4 characters, zero-padding                     |
| `Renum_flag{...; 4_SPACE}`                       | Fixed length of 4 characters, space-padding                  |

### Position Property (stores the resulting element number)

| Position Property Description                               | Behavior                                                                 |
| ----------------------------------------------------------- | ------------------------------------------------------------------------ |
| `Renum{Property:Group/Criteria_Property}`                   | Numbering by a single criterion                                          |
| `Renum{Property:Group/Criteria; Property:Group/Division}`   | Numbering with independent groups, divided by the Division Property      |

To simultaneously write the position into the **Element ID**, add to the "Description" field of the Position Property: `Sync_to{ID}`

> If a single element is selected in the project — the add-on automatically processes all elements on visible Layers with the same numbering rules. The positions of selected locked elements are considered, including elements in Hotlinks.

## Property Summation

A flag property for summation is not required. It is triggered by the "Sum Selected" command on selected elements.

| Property Description (the result is written into it)         | Action                                                                                 |
| ------------------------------------------------------------ | ---------------------------------------------------------------------------------------- |
| `Sum{Property:Group/Summable; Property:Group/Criterion}` | Sum of numbers or union of unique text values (default separator `;`) |
| `Sum{...; separator}`                                      | Set an arbitrary separator for text values                                   |
| `Sum{...; max}`                                              | Output the maximum value (only for numeric Properties)                              |
| `Sum{...; min}`                                              | Output the minimum value (only for numeric Properties)                               |

---

## Dimension Processing — Rounding and Control

Rules are set in **Project Info** (File > Info > Project Info), field `Addon_Dimensions`. Separate multiple rules with the `;` character.

### Rule Formats

```
"LAYER_NAME_PART" - PEN_MM, MODIFIED_TEXT_PEN, CONTENT_CHANGE_FLAG
DIMENSION_LINE_PEN - PEN_MM, MODIFIED_TEXT_PEN, CONTENT_CHANGE_FLAG
```

Dimension Line Pen refers to the pen of the dimension line itself (not the text). If the Flag = 0, only the text pen is changed; the Dimension's content remains unchanged.

### Additional Modifiers (added to the end of a rule via comma or space)

| Modifier       | Action                                                                 |
| -------------- | ---------------------------------------------------------------------- |
| `ClassicRound` | Classic mathematical rounding (not just rounding up)                   |
| `DeleteWall`   | Hide Wall thickness values in the dimension chain                      |
| `ResetText`    | Reset manually changed Dimension text to the measured value            |
| `CheckCustom`  | Highlight Dimensions with manually changed text                        |

Example rule: `".КЖ" - 5, 20, 0, ClassicRound`

Processing occurs: when opening a View, when changing a Story, when a tracked element is modified (if "Track" is enabled), when executing the "Synchronize All" command.

> **ArchiCAD Limitation:** Dimensions associated with Profile Columns or Openings are not processed due to an ArchiCAD bug.

---

## Filling in Changes According to GOST R 21.101-2020

The function is compatible **only** with the Change Marker and Title Block from the **kuvbur_Format by GOST** library.

- The button is active only when called from a Layout
- All visible reserved Change Markers on all Layouts in the project are processed
- **The action is irreversible** — do not reserve other users' Sheets when working collaboratively in Teamwork
- Results are written to the Layout Properties: `Somestuff_QtyIssue_1...n` and `Somestuff_Note`
- To regenerate the Sheet, switch to a different Sheet and then switch back

## QR Code from Property Value

Create a Property of type **String**. In the "Description" field, specify: `Sync_from{QRCode:Property:GROUP/PROPERTY_NAME}`

The add-on prepares a data string for the `macro_qrcode` macro (kuvbur library). The macro is called from the 2D or 3D script of a Library Element or via a ready-made Label from the kuvbur_QRCode library.

For IFC export: create an IFC property named `QRCode` (type `ifcText`), assign it a rule from the Property where the add-on has written the prepared string.

---

## Creating Specification Elements (Spec_rule)

Generates Interactive Catalog rows for composite GDL objects — overcomes the limitation of "1 element = 1 specification row".

### Property-Rule (Data Type: Matching Criterion)

```
Spec_rule {"SELECTED_NAME"; g(U1,U2; P1,P2; F; Q) s(Pn1,Pn2; Qn)}
```

- `g(...)` — source data group: `U` — uniqueness criteria, `P` — GDL Parameters or Properties to transfer, `F` — activity flag (1/0), `Q` — quantity parameters for summation
- `s(...)` — structure of the created element: `Pn` — receiving GDL Parameters or Properties, `Qn` — parameters for recording the final quantity
- `gm(...)` — analog of `g(...)` for Building Materials of Composite Construction Layers and Complex Profiles

| Rule Version | Difference                                                                          |
| ------------ | ----------------------------------------------------------------------------------- |
| `Spec_rule`    | Base version                                                                       |
| `Spec_rule_v2` | Auto-updates previously created elements + outputs changed values to Report        |
| `Spec_rule_v3` | Like v2, plus ignores elements with missing GDL parameters without error output |

The base element is taken from the active **Object** tool or from **Favorites** (if its name is specified in the rule). The base element must have Classification configured and the required Properties must be available.

---

## Performance

The add-on triggers only when an Element is modified (with "Track Changes" enabled) or when manually executed. It does not affect 3D rendering speed or performance in the 3D window. The number of synchronized Properties has virtually no impact on performance.

**Recommendations for slowdowns:**

- Restrict Property Availability by Class in the Property Manager — do not make Properties available for unnecessary Element Classes
- For bulk operations (pasting from another file, working with Hotlinks) temporarily disable "Track Changes", then perform synchronization manually
- Disable processing for Element types that are not used in the current project

---

## Diagnostics — Nothing Works

1. Is the Element **classified**? → Check the "Classifications and Properties" panel in the Element Parameters Dialog.
2. Is the Property with the description `Sync_flag` (type **Match Criteria**) **visible** in the element's parameters and set to **TRUE**?
3. Does the Element have only **one** Property with the description `Sync_flag`?
4. Are the required Properties **available** for the Element's Class and **visible** in the Parameters Dialog?
5. Is the required element type **enabled** in the add-on's menu?
6. Is the description **syntax** correct? Use curly braces `{}`, no spaces before `{`: correct `Sync_from{`, incorrect `Sync_from {`.
7. Is **Tracking enabled**? If not — select the elements and click "Synchronize Selected".
8. Open the Navigator Palette → **Report** — the add-on provides a detailed description of every operation performed and all errors.