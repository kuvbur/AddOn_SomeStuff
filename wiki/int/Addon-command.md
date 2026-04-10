# Synchronization
## Synchronization Control Flags

The flag property is a property whose description specifies Sync_flag. 

| Command | Description |
|---------|----------|
| `Sync_flag` | Property flag to disable synchronization for an element.<br>**Type:** Classification criterion. Can be calculated by formula. |
| `Sync_class_flag` | Flag to enable classification processing *(v1.72+)*.<br>If not found, `Sync_flag` is used. Other functions continue working. |
| `Sync_correct_flag` | Flag to disable coordinate accuracy check *(v1.72+)*.<br>If not found, `Sync_flag` is used. Other functions continue working. |

## Synchronization Within Object Properties

Property description

`Sync_from` - reads values FROM another location and writes TO the current property. `Sync_to` - writes the current property value to another location (property, object parameter, project info).

| Command | Description |
|---------|----------|
| `Sync_from` | Reads values from source and writes to current property |
| `Sync_to` | Writes current property value to target location |
| **Data Reading (`Sync_from`)** |  |
| `Sync_from{PARAMETER_NAME}` | Copies GDL parameter value to property (parameter name in Latin) |
| `Sync_from{PARAMETER_NAME; IGNORED_VALUES}` | Copies GDL parameter value excluding specified values |
| `Sync_from{description:PARAMETER_DESCRIPTION}` | Copies value by display name (supports Cyrillic) |
| `Sync_from{description:PARAMETER_DESCRIPTION; IGNORED_VALUES}` | Copies by display name excluding values |
| `Sync_from{Property:GROUP/PROPERTY}` | Copies value of another property |
| `Sync_from{Property:GROUP/PROPERTY; IGNORED_VALUES}` | Copies another property value excluding values |
| `Sync_from{IFC:PROPERTY_NAME}` | Copies IFC property value |
| **Data Writing (`Sync_to`)** |  |
| `Sync_to{id}` | Writes property value to element ID (not working in AC22) |
| `Sync_to{PARAMETER_NAME}` | Writes property value to GDL element parameter *(v1.6+)* |

### GDL Array Processing (Read Only) *(v1.69+)*

| Command | Description |
|---------|----------|
| `Sync_from{PARAMETER_NAME; uniq}` | Returns unique array values (text) or sum (numbers) |
| `Sync_from{PARAMETER_NAME; sum}` | Returns concatenated array values (text) or sum (numbers) |
| `Sync_from{PARAMETER_NAME; max}` | Returns maximum array value |
| `Sync_from{PARAMETER_NAME; min}` | Returns minimum array value |

To specify array range: `(FIRST_ROW,LAST_ROW)(FIRST_COLUMN,LAST_COLUMN)`.  
Example: `Sync_from{PARAMETER_NAME; uniq(1,3)}` outputs unique values from rows 1-3.  
GDL object parameters can be used: `Sync_from{PARAMETER_NAME; uniq(1,nrow)}`

### Coordinate Processing *(v1.6+)*

Processes: Windows, Doors, Walls, Beams, Columns, Objects, Zones.  
For curtain wall panels: returns panel center (x,y,z). For columns/objects: column center (x,y) and bottom elevation (z). For zones: zone center (x,y, z=0).

| Command | Description |
|---------|----------|
| **Coordinate Reading** | |
| `Sync_from{symb_pos_x}`, `Sync_from{symb_pos_y}`, `Sync_from{symb_pos_z}` | Basic element coordinates (for walls/beams - start point) |
| `Sync_from{Coord:symb_pos_sx}`, `Sync_from{Coord:symb_pos_sy}` | Element start coordinates (walls/beams) |
| `Sync_from{Coord:symb_pos_ex}`, `Sync_from{Coord:symb_pos_ey}` | Element end coordinates (walls/beams) |
| `Sync_from{Coord:symb_pos_lo_x}`, `Sync_from{Coord:symb_pos_lo_y}`, `Sync_from{Coord:symb_pos_lo_z}` | User origin relative coordinates *(v1.73+)* |
| `Sync_from{Coord:symb_pos_lo_sx}`, `Sync_from{Coord:symb_pos_lo_sy}` | Start point relative to user origin *(v1.73+, walls/beams)* |
| `Sync_from{Coord:symb_pos_lo_ex}`, `Sync_from{Coord:symb_pos_lo_ey}` | End point relative to user origin *(v1.73+, walls/beams)* |
| `Sync_from{Coord:symb_pos_sx}`, `Sync_from{Coord:symb_pos_sy}` | Absolute opening center coordinates *(v1.74, windows/doors)* |
| `Sync_from{Coord:symb_pos_lo_sx}`, `Sync_from{Coord:symb_pos_lo_sy}` | Opening center relative to user origin *(v1.74+)* |
| **Angles and Orientation** | |
| `Sync_from{Coord:symb_rotangle}` | Element rotation angle (for columns: axis + slant sum) |
| `Sync_from{Coord:symb_rotangle_slant}` | Column slant rotation angle *(v1.74+)* |
| `Sync_from{Coord:symb_rotangle_axis}` | Column section rotation angle *(v1.74+)* |
| `Sync_from{Coord:symb_rotangle_mod...}` | Integer division of angle (mod5, mod10 etc.) |
| `Sync_from{Coord:north_dir}` | Angle between element and project north |
| `Sync_from{Coord:north_dir_str}` | Cardinal direction (string) *(v1.69+)* |
| `Sync_from{Coord:north_dir_eng}` | Cardinal direction (English) *(v1.70+)* |
| **Coordinate Writing** | |
| `Sync_to{Coord:symb_pos_x}`, `Sync_to{Coord:symb_pos_y}`, `Sync_to{Coord:symb_pos_z}` | Write element coordinates *(v1.72+, objects/columns)* |
| `Sync_to{Coord:symb_pos_sx}`, `Sync_to{Coord:symb_pos_sy}` | Write start coordinates *(v1.72+, walls/beams)* |
| `Sync_to{Coord:symb_pos_ex}`, `Sync_to{Coord:symb_pos_ey}` | Write end coordinates *(v1.72+, walls/beams)* |
| `Sync_to{Coord:symb_rotangle}` | Write rotation angle *(v1.72+)* |
| **Special Functions** | |
| `Sync_from{Coord:locorigin_x}`, `Sync_from{Coord:locorigin_y}`, `Sync_from{Coord:locorigin_z}` | User origin coordinates *(v1.73+)* |
| `Sync_from{Coord:windoor_in_wall}` | Check if opening is in wall *(v1.74+, False = outside wall)* |

**Fractional Part Check for Coordinates/Angles**  
[YouTube example](https://youtu.be/a9Dnvo1P-rk)  
[Rutube example](https://rutube.ru/video/c853151ae9a8b3582bd3a10fb52863f9)  

Functions return TRUE if fractional part is absent, FALSE if exceeds tolerance.  
Tolerances: 0.001 mm for coordinates/length, 0.00001° for angles.  

Disable check by:  
1. Setting `Sync_flag` to FALSE (disables all sync)  
2. Creating boolean property `Sync_correct_flag` = FALSE (disables only fractional check)  

| Command | Description |
|---------|----------|
| **Basic Checks** | |
| `Sync_from{Coord:symb_pos_correct}` | General start/end X/Y check |
| `Sync_from{Coord:symb_pos_x_correct}`, `Sync_from{Coord:symb_pos_y_correct}` | X/Y coordinate check |
| `Sync_from{Coord:symb_pos_sx_correct}`, `Sync_from{Coord:symb_pos_sy_correct}` | Start point check (walls/beams) |
| `Sync_from{Coord:symb_pos_ex_correct}`, `Sync_from{Coord:symb_pos_ey_correct}` | End point check (walls/beams) |
| `Sync_from{Coord:symb_rotangle_fraction}` | Rotation angle fractional part |
| `Sync_from{Coord:symb_rotangle_correct}` | Fractional part check (tolerance 0.00001°) |
| `Sync_from{Coord:symb_rotangle_correct_1000}` | Fractional part check (tolerance 0.001°) |
| **Strict Checks (0.000001 mm tolerance)** | |
| `Sync_from{Coord:symb_pos_correct_hard}` | General start/end check |
| `Sync_from{Coord:symb_pos_x_correct_hard}`, `Sync_from{Coord:symb_pos_y_correct_hard}` | Strict X/Y check |
| `Sync_from{Coord:symb_pos_sx_correct_hard}`, `Sync_from{Coord:symb_pos_sy_correct_hard}` | Strict start point check |
| `Sync_from{Coord:symb_pos_ex_correct_hard}`, `Sync_from{Coord:symb_pos_ey_correct_hard}` | Strict end point check |
| **User Origin Relative Checks** | |
| `Sync_from{Coord:symb_pos_lo_correct}` | General start/end check *(v1.73+)* |
| `Sync_from{Coord:symb_pos_lo_x_correct}`, `Sync_from{Coord:symb_pos_lo_y_correct}` | X/Y check *(v1.73+)* |
| `Sync_from{Coord:symb_pos_lo_sx_correct}`, `Sync_from{Coord:symb_pos_lo_sy_correct}` | Start point check *(v1.73+, walls/beams)* |
| `Sync_from{Coord:symb_pos_lo_ex_correct}`, `Sync_from{Coord:symb_pos_lo_ey_correct}` | End point check *(v1.73+, walls/beams)* |
| `Sync_from{Coord:symb_pos_lo_correct_hard}` | Strict general check *(v1.73+)* |
| `Sync_from{Coord:symb_pos_lo_x_correct_hard}`, `Sync_from{Coord:symb_pos_lo_y_correct_hard}` | Strict X/Y check *(v1.73+)* |
| `Sync_from{Coord:symb_pos_lo_sx_correct_hard}`, `Sync_from{Coord:symb_pos_lo_sy_correct_hard}` | Strict start point check *(v1.73+, walls/beams)* |
| `Sync_from{Coord:symb_pos_lo_ex_correct_hard}`, `Sync_from{Coord:symb_pos_lo_ey_correct_hard}` | Strict end point check *(v1.73+, walls/beams)* |
| **Special Checks** | |
| `Sync_from{Coord:has_distant_element}` | Distant elements check *(v1.76+)* |

### Project Location Data *(v1.6+)*

* `Sync_from{Glob:GLOB_NORTH_DIR}`
* `Sync_from{Glob:GLOB_PROJECT_LONGITUDE}`
* `Sync_from{Glob:GLOB_PROJECT_LATITUDE}`
* `Sync_from{Glob:GLOB_PROJECT_ALTITUDE}`
* `Sync_from{Glob:GLOB_SUN_AZIMUTH}`
* `Sync_from{Glob:GLOB_SUN_ALTITUDE}`

### Writing to Project Info *(v1.6+)*

* `Sync_to{Info:PROPERTY_NAME}` - Writes property value to custom project info property. Works only during full or selected elements sync.

## Morph Line Processing *(v1.6+)*

| Command | Description |
|---------|----------|
| `Sync_from{Morph:L}` | Morph line total length |
| `Sync_from{Morph:Lx}`, `Sync_from{Morph:Ly}`, `Sync_from{Morph:Lz}` | Length along X/Y/Z axes |
| `Sync_from{Morph:Max_x}`, `Sync_from{Morph:Max_y}`, `Sync_from{Morph:Max_z}` | Maximum coordinates |
| `Sync_from{Morph:Min_x}`, `Sync_from{Morph:Min_y}`, `Sync_from{Morph:Min_z}` | Minimum coordinates |
| `Sync_from{Morph:A}`, `Sync_from{Morph:B}`, `Sync_from{Morph:ZZYZX}` | Bounding box dimensions (like library elements) |

### [Material Composition](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%92%D1%8B%D0%B2%D0%BE%D0%B4-%D1%81%D0%BE%D1%81%D1%82%D0%B0%D0%B2%D0%B0-%D0%BA%D0%BE%D0%BD%D1%81%D1%82%D1%80%D1%83%D0%BA%D1%86%D0%B8%D0%B8)

For custom material properties, add `{@property:buildingmaterialproperties}` to description

| Command | Description |
|---------|----------|
| `Sync_from{Material:Layers; "TEMPLATE"}` | Outputs layer composition |
| `Sync_from{Material:Layers,PEN_NUMBER; "TEMPLATE"}` | Profile composition at pen line *(v1.6+)* |
| `Sync_from{Material:Layers_inv; "TEMPLATE"}` | Reverse layer order *(v1.70+)* |
| `Sync_from{Material:Layers_auto; "TEMPLATE"}` | Auto layer order (as in preview) *(v1.70+)* |
| `Sync_from{Material:Layers; "TEMPLATE <FORMULA>"}` | Output with expression calculation *(v1.72+)* |
| `Sync_from{Material:Layers,all; "TEMPLATE"}` | Output all layers (auto thickness) *(v1.77+)* |
| `Sync_from{Material:Layers,unic; "TEMPLATE"}` | Output unique layers (merge identical) *(v1.77+)* |

## Layer Change *(v1.72+)*

* `Sync_to{Attribute:Layer}` - Searches by full layer name (with extension)

## Element Information *(v1.77+)*

* `Sync_from{Element:Material overridden}` - Checks if material override is enabled

## MEP Element Information *(v1.77+)*
Processes pipes, ducts, transitions, bends, routes. 
[Example АС25](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/files/MEP_28.pln)
| Command | Description |
|---------|----------|
| `Sync_from{MEP:reference set name}` | Main diameter table name |
| `Sync_from{MEP:element set name}` | Element table name (pipe/bend/transition) |
| `Sync_from{MEP:width}` | Section width |
| `Sync_from{MEP:height}` | Section height |
| `Sync_from{MEP:wall thickness}` | Wall thickness (pipes/ducts) |
| `Sync_from{MEP:bend radius}` | Bend radius |
| `Sync_from{MEP:description}` | Description from settings table |
| `Sync_from{MEP:shape}` | Section shape |
| `Sync_from{MEP:length}` | Element length (not for bends) |
| `Sync_from{MEP:insulation thickness}` | Insulation thickness |
| `Sync_from{MEP:system}` | System name |
| `Sync_from{MEP:routing length}` | Containing route segment length |

## [Hierarchical Structure Sync](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%A1%D0%B8%D0%BD%D1%85%D1%80%D0%BE%D0%BD%D0%B8%D0%B7%D0%B0%D1%86%D0%B8%D1%8F-%D1%81%D0%B2%D0%BE%D0%B9%D1%81%D1%82%D0%B2-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%B0-%D1%81%D0%BE-%D1%81%D0%B2%D0%BE%D0%B9%D1%81%D1%82%D0%B2%D0%B0%D0%BC%D0%B8-%D0%B4%D1%80%D1%83%D0%B3%D0%B8%D1%85-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%BE%D0%B2)

Uses `_sub` suffix (e.g., `Sync_from_sub{Property:GROUP/PROPERTY}`) for:  
- Curtain walls  
- Elements in zones  
Works same as single-element sync but for nested elements.

## Property Reset to Default

* `Sync_reset` - Resets property everywhere (placed elements, favorites, layouts). Sync disabled while `Sync_reset` is in description. *Not working in AC27*

# [Renumbering](https://github.com/kuvbur/AddOn_SomeStuff/wiki/%D0%9F%D0%B5%D1%80%D0%B5%D0%BD%D1%83%D0%BC%D0%B5%D1%80%D0%B0%D1%86%D0%B8%D1%8F-%D1%8D%D0%BB%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%BE%D0%B2)

*(v1.74+)* If only one element selected: processes all visible layer elements with available numbering rules.  
Auto-syncs selected elements after renumbering.

**Numbering Flag Formats**
* `Renum_flag{rule_property_name}`
* `Renum_flag{rule_property_name ; NULL}`
* `Renum_flag{rule_property_name ; SPACE}` *(v1.6+)*
* `Renum_flag{rule_property_name ; ALLNULL}`

**Position Rule Property Formats**
* `Renum{criteria_property_name}`
* `Renum{criteria_property_name; grouping_property_name}`

# Summation
Sums property values for elements with same criteria property.  
Numbers are summed, text: unique values joined (default separator: semicolon).  
No flag required. Auto-sync after summation.  
*(v1.74+)* If one element selected: processes all visible layer elements with summation rules.

* `Sum{Property:GROUP/SUM_PROPERTY; Property:GROUP/CRITERIA_PROPERTY}`
* `Sum{Property:GROUP/SUM_PROPERTY; Property:GROUP/CRITERIA_PROPERTY; separator}` - Custom separator
* `Sum{Property:GROUP/SUM_PROPERTY; Property:GROUP/CRITERIA_PROPERTY; max}` - Max value *(v1.72+)*
* `Sum{Property:GROUP/SUM_PROPERTY; Property:GROUP/CRITERIA_PROPERTY; min}` - Min value *(v1.72+)*  
![Summation example](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/sum.PNG)  
![Summation example 2](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/sum_2.PNG)

## Element Classification *(v1.72+)*

* Add `some_stuff_class` to class description to enable classification
* `Sync_to{Class:CLASSIFICATION_NAME}` - Assigns class (searches by class ID)
* `Sync_from{Class:CLASSIFICATION_NAME; FullName}` - Outputs full class name
### Enabling Classification for Unclassified Elements
Automatic class assignment for unclassified elements  

Add the value to the class description: `some_stuff_class`  
*Configuration example:*  
![image](https://github.com/user-attachments/assets/b87efbe3-094c-4f9a-895d-57a7bf38cc2b)

### Changing Class Based on Property Value
Dynamic element class update when linked property changes  

1. Create a text property  
2. In the property description specify:  
   `Sync_to{Class:CLASSIFICATION_NAME}`  
   *(Class search is performed by its ID)*  
3. Add your formula to the property that will output the class ID

*Configuration example:*  
![image](https://github.com/user-attachments/assets/2e1c08cb-1b36-4a4a-8cae-1f6a6da39f69)

### Displaying Class Information
Create a property with description:  
`Sync_from{Class:CLASSIFICATION_NAME; FullName}`  
- Outputs element's full class name  

## Disabling Classification Processing
**Options:**  
- Using main processing flag:  
  Property `Sync_flag` (completely disables element processing)  
- Creating specialized flag:  
  Property with description `Sync_class_flag` (disables only classification processing)  

## Usage Examples  
- [Youtube demonstration video](https://youtu.be/xTOY58xC4Tg)  
- [Rutube demonstration video](https://rutube.ru/video/87890fab67db3b1c2d2d7067a37e0475)


# Dimension Rounding

**Due to a bug in Archicad, dimensions attached to profile columns or openings are NOT processed!**

All linear, reserved, visible, and unlocked dimensions on the current view that match the rules specified in the `Addon_Dimensions` project info field are processed. Write rules with separator ;  

![dim_rule.PNG](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/dim_rule.PNG)  

Checking and processing of all dimensions on the current view occurs:  
* When opening any view (switching active window)  
* When changing stories  
* Any action with tracked elements (when "Track" is enabled in add-on menu)  
* When running the `Synchronize All` command  

Possible values:  
* `"LAYER" - ROUNDING_MM, MODIFIED_TEXT_PEN, CONTENT_CHANGE_FLAG`  
* `"LAYER" - ROUNDING_MM, MODIFIED_TEXT_PEN, <FORMULA>`  
* `"LAYER" - ROUNDING_MM, MODIFIED_TEXT_PEN, CONTENT_CHANGE_FLAG, <FORMULA>`  
* `DIMENSION_PEN - ROUNDING_MM, MODIFIED_TEXT_PEN, CONTENT_CHANGE_FLAG`  
* `DIMENSION_PEN - ROUNDING_MM, MODIFIED_TEXT_PEN, <FORMULA>`  
* `DIMENSION_PEN - ROUNDING_MM, MODIFIED_TEXT_PEN, CONTENT_CHANGE_FLAG, <FORMULA>`  

`DIMENSION_PEN` refers to the dimension line's pen  

![dim_pen.PNG](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/dim_pen.PNG)  

Rounding is performed upwards. _v1.77_ For classic rounding, add `ClassicRound` at the end of the rule, e.g. `".КЖ" - 5, 20, 0, ClassicRound`  

When specifying a layer name, the system checks if the rule's text fragment is contained in the layer name. All dimensions not divisible by the specified value change their color to "MODIFIED_TEXT_PEN". If the content change flag is 0, text replacement isn't performed - only the pen changes.  

Example: `".КЖ" - 5, 20, 0` will highlight (but not modify) all dimensions not divisible by 5mm on layers containing ".КЖ" in red (pen 20).  

Formulas may use property values associated with dimension elements.  

_v1.72_ To hide wall thicknesses, add `DeleteWall` to the rule  
_v1.72_ To reset dimension chain values to measured values, add `ResetText` to the rule. Only resets values, pen remains unchanged.  
_v1.76_ To highlight overridden dimensions, add `CheckCustom` to the rule  