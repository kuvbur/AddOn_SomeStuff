## Finishing Schedule

[Test file](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/files/%D0%9E%D1%82%D0%B4%D0%B5%D0%BB%D0%BA%D0%B0_25.pla)

Performs a breakdown by the composition of adjacent structures, recording information about the layers of adjacent structures.

Can be used both for constructing additional finishing elements on top of existing structures (similar to the Interior Master) and for recording finishing information in the zone properties. These two mechanisms work independently - they can be used either individually or together.

### Limitations

- Does not account for zone trimming by roofs
- Does not work with solid element operations on zones/bounding structures
- For reveals, uses only data from window parameters (not from zone calculation settings)
- Does not process stairs

### Building Material Properties

The add-on analyzes wall layers with the "Finish" type, collecting data from the zone outward to the first "Core" layer (inclusive). **Note:** in constructions without a "Core" layer, ALL layers are output.

| Property                         | Type                  | Description                                                                                                                                                                |
| -------------------------------- | --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `some_stuff_layer_onoff`         | Matching Criterion    | Output layer to the finishing schedule                                                                                                                                          |
| `some_stuff_layer_has_finish`    | Matching Criterion    | Adding a finish layer from zone parameters (optional). When disabled, the finish layer from the zone is replaced by the material specified in the building material settings. |
| `some_stuff_layer_description`   | Text                  | Material name. Supports templates with `%%`                                                                                                                         |
| `some_stuff_layer_favorite_name` | Text                  | Favorite name (optional)                                                                                                                                            |

If properties are absent, the names of all finishing layers are output.

### Zone Properties

The created finishing walls will be divided in height according to the settings of the zone properties. There are three possible sections in total - panels (made of walls), the main part, and the top strip (which can be used for the space from the suspended ceiling to the slab).

#### Zone Properties for Reading

#### Element Creation Control

| Property Description             | Purpose                                  |
| -------------------------------- | ---------------------------------------- |
| `some_stuff_fin_create_elements` | main toggle for element creation         |
| `some_stuff_fin_create_ceil`     | create ceilings                          |
| `some_stuff_fin_create_floor`    | create floors                            |
| `some_stuff_fin_create_wall`     | create walls                             |
| `some_stuff_fin_create_column`   | create columns                           |
| `some_stuff_fin_create_reveal`   | create reveals                           |

Property type - matching criterion. If specialized properties are absent - the value of `some_stuff_fin_create_elements` is used. If properties are not set - elements will be created.

#### Finishing Configuration

When using the add-on together with the [library](https://github.com/kuvbur/gdl_bibl), finishing properties are read directly from the Zone parameters.
When using other zones, it is necessary to create properties with the following descriptions (i.e., create a property and add the required text to its description):

| Property Description            | Purpose                                                                                    | Data Type            |
| -------------------------------- | --------------------------------------------------------------------------------------------- | --------------------- |
| `some_stuff_fin_ceil_material`   | Ceiling finish material name (final finish)                                                       | String                |
| `some_stuff_fin_up_material`     | Upper part finish material name (difference between zone height and main finish height, final finish) | String                |
| `some_stuff_fin_main_material`   | Main wall part finish material name (final finish)                                          | String                |
| `some_stuff_fin_down_material`   | Panel finish material name (bottom of walls and columns, final finish)                                   | String                |
| `some_stuff_fin_column_material` | Column finish material name (final finish)                                                        | String                |
| `some_stuff_fin_main_height`     | Zone height up to the ceiling (if not set or equals zero - the zone height is used)             | Length                 |
| `some_stuff_fin_down_height`     | Panel height                                                                                | Length                 |
| `some_stuff_fin_has_ceil`        | Presence of a ceiling                                                                               | Criteria |
| `some_stuff_fin_floor_by_slab`   | Calculate and build ceiling only based on slabs                                               | Criteria |
| `some_stuff_fin_has_floor`       | Presence of a floor                                                                                  | Criteria |
| `some_stuff_fin_ceil_by_slab`    | Calculate and build floor only based on slabs                                                   | Criteria |
| `some_stuff_fin_type`            | Finishing type for merging zones with identical finishes                                         | String                |

#### Zone Properties for Result Recording

Used for outputting results without constructing finishing elements. All finishing layers are recorded into a single cell.
The property type for result recording is string. If one of the mandatory properties is not found, recording to the others is not performed. Area calculation is performed without considering intersections/subtractions of finishing walls.

| Property Description                              | Purpose                                              | Note                              |
| -------------------------------------------------- | ------------------------------------------------------- | --------------------------------------- |
| `some_stuff_fin_ceil_result`                       | Ceiling finish                                         | Mandatory property                   |
| `some_stuff_fin_up_result`                         | Upper wall finish, from suspended ceiling to top of wall | If not specified - will be added to the wall column |
| `some_stuff_fin_main_result`                       | Wall finish                                            | Mandatory property                   |
| `some_stuff_fin_down_result`                       | Lower wall/column finish                                | Mandatory property                   |
| `some_stuff_fin_column_result`                     | Column finish                                          | Mandatory property                   |
| `some_stuff_fin_reveal_result`                     | Reveal finish                                         | If not specified - will be added to the wall column |
| Result recording with grouping by finish type |
| `some_stuff_fin_up_result_bytype`                  | Upper wall finish, from suspended ceiling to top of wall | If not specified - will be added to the wall column |
| `some_stuff_fin_main_result_bytype`                | Wall finish                                            | Mandatory property                   |
| `some_stuff_fin_down_result_bytype`                | Lower wall/column finish                                | Mandatory property                   |
| `some_stuff_fin_column_result_bytype`              | Column finish                                          | Mandatory property                   |
| `some_stuff_fin_reveal_result_bytype`              | Reveal finish                                         | If not specified - will be added to the wall column |

#### Output Formatting (Font, Font Size)

Since all materials along with the area are written into one cell, the add-on adjusts line breaks based on data about the font and the expected column widths. By default, the add-on is configured to work with GOST 2.304 type A font, font size 3, width for material 40mm, for area 20mm (total column width in the IK - 60mm). To specify other values, add them to the description of the desired output column in the format `{MATERIAL_WIDTH ; AREA_WIDTH ; SIZE ; FONT}`, for example `{ 50 ; 30 ; 3 ; ISOCPEUR }`

## Settings for Created Finishing Elements

These settings are required only for creating finishing elements and do not affect writing the finishing schedule to the zone.

### Classification of Finishing Elements

For the add-on to function, it is necessary to configure the classes for finishing elements. The configuration involves adding the following text to the Class Description:

- `some_stuff_fin_class` - General class for all elements. Assigned if other classes are not found.
- `some_stuff_fin_walls` - For walls (if not specified, it will take the `some_stuff_fin_class`)
- `some_stuff_fin_down_walls` - For wall bases (if not specified, it will take the `some_stuff_fin_class`)
- `some_stuff_fin_reveals` - For reveals (if not specified, it will take the `some_stuff_fin_wall` or `some_stuff_fin_class`)
- `some_stuff_fin_columns` - For columns (if not specified, it will take the `some_stuff_fin_wall` or `some_stuff_fin_class`)
- `some_stuff_fin_floors` - For floors (if not specified, it will take the `some_stuff_fin_class`)
- `some_stuff_fin_ceils` - For ceilings (if not specified, it will take the `some_stuff_fin_class`)
- `some_stuff_fin_skirting` - For floor skirting (dedicated class, no fallback)
- `some_stuff_fin_cornice` - For ceiling cornice (dedicated class, no fallback)

For the add-on to work correctly, the existence of the class with the description `some_stuff_fin_class` is sufficient, except for skirting and cornice - they require dedicated classes; other classes can be created as needed. The add-on itself assigns the classification to elements during the creation of finishing elements; manual assignment is not required. Elements classified as finishing are not considered in calculations.

### Properties of Finishing Elements

Properties must be visible in the classification of finishing elements

| Property | Data type | Description |
| --- | --- | --- |
| `Sync_GUID base element` | String, single value | For the add-on to record the GUID of the base element to which the finishing element will be linked |
| `Sync_GUID zone` | String, single value | For the add-on to record the GUID of the zone |

For common roles `Sync_GUID zone` is mandatory - without it the calculation stops with a configuration error. A missing `Sync_GUID base element` in common roles only produces a warning: existing floor/ceiling elements will be updated by replacement.

For skirting and cornice the check is strict: both properties must be of type `String`, collection `Single value`, and explicitly available in the corresponding role class; otherwise the calculation ends with a configuration error.

### Skirting and Cornice

Floor skirting and ceiling cornice are separate roles (`Floor_Skirting`, `Ceil_Cornice`) that are created only from Beam elements.

- They use dedicated classes `some_stuff_fin_skirting` and `some_stuff_fin_cornice`; fallback to `some_stuff_fin_class` is not applied.
- If the role class is missing or not recognized, the calculation ends with a configuration error.
- Properties `Sync_GUID zone` and `Sync_GUID base element` are mandatory and must be a string with a single value, available in the role class.

### Properties of Base Elements

- `some_stuff_element_has_finish` - instead of applying the zone's finish, the material (coating) of the element's last layer will be applied.
- `some_stuff_element_onoff` - completely disables the element's processing by the finish schedule.

### Creating Finishing Elements from Favorites

The add-on checks whether the name of a finishing material in a zone contains a reference to a favorite name. The material name is formed in the format `MATERIAL_NAME@FAVORITE_NAME`, for example `Paint Coating@Wall` - where `Paint Coating` is the material name in the finishing schedule, and `Wall` is the name of the element in the favorites.

If no favorite is specified for the material (i.e., the material name does not contain the `@` symbol or a favorite with that name is not found), the add-on will search for favorite elements based on the type of surface being finished:

- `smstf window` - opening for insertion into finishing walls at window and door locations. Tool type - window
- `smstf wall` - finishing of vertical surfaces. Tool type - wall or object (accessory)
- `smstf ceil` - for modeling ceilings. Tool type - slab or object (accessory)
- `smstf floor` - for modeling floors. Tool type - slab or object (accessory)
- `smstf reveal side` - for modeling the vertical part of reveals. Tool type - wall or object (accessory)
- `smstf reveal up` - for modeling the horizontal part of reveals. Tool type - beam
- `smstf ceil side` - for walls at ceiling height changes. Tool type - wall or object (accessory)
- `smstf floor side` - for walls at floor height changes. Tool type - wall or object (accessory)
- `smstf skirting` - for floor skirting. Tool type - beam only; fallback to `smstf wall` or material is not applied
- `smstf cornice` - for ceiling cornice. Tool type - beam only; fallback to `smstf wall` or material is not applied

If no suitable elements are found in the favorites at all, the tool's default settings will be applied (for walls and slabs). This fallback does not apply to skirting and cornice.

## RoomBook JSON command

The command is invoked through the HTTP port of the running Archicad (`POST http://127.0.0.1:19723`) and takes no calculation parameters: `addOnCommandParameters` can be omitted or passed as an empty object.

### Request

```json
{
  "command": "API.ExecuteAddOnCommand",
  "parameters": {
    "addOnCommandId": {
      "commandNamespace": "SomeStuffCommand",
      "commandName": "RoomBook"
    },
    "addOnCommandParameters": {}
  }
}
```

### Response

Archicad wraps the command result in `result.addOnCommandResponse`:

```json
{
  "succeeded": true,
  "result": {
    "addOnCommandResponse": {
      "status": "returned",
      "rooms": [
        {
          "zone_guid": "B69820A5-9B3A-4D30-A33A-DE995E4447CB",
          "types": [
            {
              "TypeOtd": 4,
              "elements": [
                {
                  "otd_guid": "93A36784-485B-4D8C-AB6B-F11BB4E94A7A",
                  "base_guid": "61AEC870-2A86-42BB-82E6-8A3FF809CA2F",
                  "write_succeeded": true,
                  "was_created": true,
                  "length": 0.02,
                  "length_available": true,
                  "area": 0.042,
                  "area_available": true,
                  "material": 366,
                  "bmaterial": 3,
                  "structtype": 0
                }
              ]
            }
          ]
        }
      ]
    }
  }
}
```

Response fields:

| Field | Type | Description |
| --- | --- | --- |
| `status` | string | Call completion marker; the current implementation always returns `returned` |
| `rooms` | array | Processed rooms (zones) |
| `rooms[].zone_guid` | string | Room GUID |
| `rooms[].types` | array | Created finishing elements grouped by type |
| `types[].TypeOtd` | number | Numeric finishing type (`TypeOtd`), see the table below |
| `types[].elements` | array | Finishing elements of this type in the room |
| `elements[].otd_guid` | string | GUID of the created or updated finishing element |
| `elements[].base_guid` | string | GUID of the base element the finishing belongs to |
| `elements[].write_succeeded` | bool | The element was successfully written to the model |
| `elements[].was_created` | bool | The element was created in this run (`false` - an existing one was updated) |
| `elements[].length` | number | Linear length (for roles where it is calculated) |
| `elements[].length_available` | bool | `length` has been calculated |
| `elements[].area` | number | Element area |
| `elements[].area_available` | bool | `area` has been calculated |
| `elements[].material` | number | Finishing material index |
| `elements[].bmaterial` | number | Building material index |
| `elements[].structtype` | number | Structure type |

`TypeOtd` values:

| Value | Finishing type |
| --- | --- |
| 0 | Not set (`NoSet`) |
| 1 | Main zone finish (`Wall_Main`) |
| 2 | Upper zone finish (`Wall_Up`) |
| 3 | Lower zone finish (`Wall_Down`) |
| 4 | Reveal side (`Reveal_Main`) |
| 5 | Reveal top (`Reveal_Up`) |
| 6 | Reveal bottom (`Reveal_Down`) |
| 7 | Column finish (`Column`) |
| 8 | Floor finish (`Floor`) |
| 9 | Ceiling finish (`Ceil`) |
| 10 | Floor skirting (`Floor_Skirting`) |
| 11 | Ceiling cornice (`Ceil_Cornice`) |
| 12 | Ceiling beam finish (`Beam_Ceil`) |
| 13 | Wall beam finish (`Beam_Wall`) |

The response does not guarantee that every listed element remains in the model: the write batch can be rolled back, so rely on `write_succeeded` and `was_created`.
