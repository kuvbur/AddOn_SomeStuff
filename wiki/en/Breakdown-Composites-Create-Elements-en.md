## Function 'Create Elements'

Creates specification rows for composite GDL objects, bypassing the "one object = one row" limitation by generating additional elements.

## Which Elements Are Processed?

The plugin analyzes elements in the following priority order:

1.  **Selected Elements**  
    Only selected objects are processed (in the report: `Create spec from selection`)
2.  **Default Element Properties**  
    Without a selection - the active `Object` tool is checked. If a rule exists, all visible elements with this property are processed (in the report: `Create spec from default element`)
3.  **All Visible Elements**  
    Without a selection and without rules for the default element - all rules for all visible elements are processed (in the report: `Create spec from all visible element`)

## Creating a Specification Rule

1.  **Base Element**  
    Uses the object from the active `Object` tool:
    - _v1.74+:_ If `ИМЯ_ИЗБРАННОГО` is present in the rule, the Favorite element is used
2.  **Add a Property**  
    Create a custom property of type **Matching Criteria**
3.  **Description Format:**  
    `Spec_rule {ИМЯ_ИЗБРАННОГО;g(U1,U2,U3; P1,P2,P3; F1; Q1,Q2) g(...) s(Pn1,Pn2,Pn3; Qn1,Qn2)}`  
    Where:
    - `g(...)` **Source Data Group**:
      - `U1,U2,U3`: Grouping parameters (unique combinations) _v1.78+:_ If a dash is used, the parameters for transfer will be used as unique
      - `P1,P2,P3`: Parameters to transfer to the new element
      - `F1`: Activity flag (`1`/`0`, default is `1`)
      - `Q1,Q2`: Quantity parameters for summation (default is `1`)
    - `s(...)` **New Element Structure**:
      - `Pn1,Pn2,Pn3`: Data receiving parameters
      - `Qn1,Qn2`: Parameters for writing quantities

**Important:**

- The number of `P` must match the number of `Pn`
- Commas `,` and semicolons `;` are prohibited in parameter names
- Reading array parameters is supported

## Operating Principle

1.  Collect elements (by higher priorities first)
2.  Filter by `F=1`
3.  Group by unique combinations of `U`
4.  For each group:
    - Read values of `P`
    - Sum values of `Q`
5.  Create new elements:
    - Write values of `P` to parameters `Pn`
    - Write sums of `Q` to parameters `Qn`

## Additional Properties

- **Recording GUID of Source Elements**  
  Add the property: `Sync_GUID {Property:RULE_NAME_PROPERTY}`  
  _Required for highlighting the source (parent) elements with the "Show Related Elements" command_
- **Recording Rule Name**  
  Add the property: `Spec_rule_name`  
  _Allows filtering created elements in the Interactive Catalog_

## Output of Component Quantities for System Elements (Wall, Column, etc.) _1.77+_

gm(%nosyncname%%hatching% - %th%mm; %hatching% - %th%mm, %unit% ; %qty%; %qty%)
    Where:
    - `gm(...)` **Source Data Group for Material**:
        - `%nosyncname%%hatching% - %th%mm`: Grouping parameters (unique combinations)
        - `%hatching% - %th%mm`, `%unit%`: Parameters to pass to the new element
        - `%qty%`: Activity flag
        - `%qty%`: Quantity parameters for summation

## Version 2 of the _1.77+_ Rule

Start the rule with `Spec_rule_v2`, otherwise the syntax remains the same.
Features:

- Automatically updates already placed elements, outputting a report of changed values.
  During operation, it finds previously placed elements (search is performed by the Rule Name, across ALL layers and stories). It deletes obsolete elements, changes and highlights modified elements, and creates new ones. Information about changes is output to a report.
  **IMPORTANT! The search for created elements occurs either within the selection (if selected elements are being specified) or within all elements.**

## Third Version of Rule _1.77+_

Start the rule with `Spec_rule_v3`, otherwise the syntax remains the same.
Features:

- If parameters are not found for an element, it does not output errors but ignores that element.
- Automatically updates already placed elements with a report of the changed values

## Possible Errors

| Error                                                                                             | Cause                                                                                                                                                                                                                                 | Solution                                                                                                                                                                   |
| -------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Required properties are missing in the default or Favorite element**                | Rules processed successfully, but the target object lacks required properties when creating elements. Most often caused by incorrect or missing classification of the favorite or default element.                 | 1. Check the **Classification** in the default element<br>2. Ensure properties exist in the settings<br>3. For Favorites, enable "Transfer properties and classifications" |
| **Specification rule found, but the rule property is disabled (not True) for elements on the plan** | The rule property has a value of `False` or is inactive for the source elements.                                                                                                                                                               | Check the value of the rule property for elements on the plan.                                                                                                                  |
| **Failed to find parameters/properties specified in the rule**                                       | Parameters `U`, `P`, `Q`, or `F` from the rule are missing from the source elements. Possibly, some elements are classified incorrectly. For example, some elements have an available and enabled specification rule that does not apply to them. | 1. Check element classification<br>2. Verify parameter names in the rule and properties.                                                                                 |
| **Specification rules not found**                                                                | The default element lacks properties with rules.                                                                                                                                                                                | Add a property of type **Matching Criterion** with a rule to the default element settings.                                                                             |

## Rule String Examples

| Rule                                                                                                                                                                                                                                                                                                                                                                                                                   | Description                                                                                                                                                       |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `Spec_rule {"АР_Спец_Перемычки"; g(perem_naen[4],Property:АР_Перемычки/Собственный этаж;param_name_out[4], perem_obozn[4], perem_naen[4], perem_ves[4],Property:АР_Перемычки/Собственный этаж;perem_nagr[4];perem_nagr[4]) s(pos,Property:АР_Перемычки/Обозначение,Property:АР_Перемычки/Наименование,Property:АР_Перемычки/Масса ед,Property:АР_Перемычки/Собственный этаж;Property:АР_Перемычки/Количество (на этаж))}` | Lintel schedule with breakdown by floors. The breakdown is performed by the name `perem_naen[4]` and the value of the property 'Property:АР_Перемычки/Собственный этаж' |
| `Spec_rule_v2 {""; gm(%nosyncname%%штриховка% - %th%мм; %штриховка% - %th%мм, %unit% ; %qty%; %qty%, %qty%) s(Property:Спецификация материалов/Наименование, Property:Спецификация материалов/Ед. изм.; Property:Спецификация материалов/Количество, Property:Спецификация материалов/Формула)}`                                                                                                                          | Material schedule                                                                                                                                        |

## Specification for kuvbur Objects _1.78+_

### Assemblies

| Name              | Description      |
| ---------------- | ------------- |
| `%subpos.pos%`   | Position       |
| `%subpos.obozn%` | Designation   |
| `%subpos.naen%`  | Name  |
| `%subpos.qty%`   | Quantity    |
| `%subpos.ves%`   | Unit Mass     |
| `%subpos.units%` | Unit of Measurement |

### Reinforcement

| Name           | Description                                                                 |
| -------------- | --------------------------------------------------------------------------- |
| `%arm.pos%`    | Position                                                                    |
| `%arm.klass%`  | Class                                                                       |
| `%arm.diam%`   | Diameter (without the φ symbol)                                             |
| `%arm.qty%`    | Quantity (if output in linear meters is specified, the total length is displayed) |
| `%arm.dlin%`   | Length                                                                      |
| `%arm.ves_t%`  | Linear weight                                                               |
| `%arm.ves%`    | Weight (if output in linear meters is specified, the linear weight is displayed) |
| `%arm.unit%`   | Unit of measurement (`pcs` or `m`), for bent bars an asterisk \* is added   |

### Rolled Steel

| Name                    | Description                                                                 |
| ---------------------- | --------------------------------------------------------------------------- |
| `%prokat.pos%`         | Position                                                                    |
| `%prokat.tip_konstr%`  | Construction Type                                                           |
| `%prokat.obozn_mater%` | Steel GOST Standard                                                         |
| `%prokat.mater%`       | Steel                                                                       |
| `%prokat.obozn%`       | Profile GOST Standard                                                       |
| `%prokat.tip_profile%` | Profile                                                                     |
| `%prokat.qty%`         | Quantity (if the profile specifies output in linear meters, total length is output) |
| `%prokat.dlin%`        | Length                                                                      |
| `%prokat.ves_t%`       | Linear Weight                                                               |
| `%prokat.ves%`         | Weight (if the profile specifies output in linear meters, linear weight is output) |
| `%prokat.units%`       | Unit of Measurement (`pcs` or `m`)                                          |

### Materials (profile painting, etc.), products

| Name               | Description                          |
| ------------------ | --------------------------------- |
| `%mat.pos%`        | Position                           |
| `%mat.tip_konstr%` | Construction type (for KM coatings) |
| `%mat.obozn%`      | Designation                       |
| `%mat.naen%`       | Name                      |
| `%mat.qty%`        | Quantity                        |
| `%mat.ves%`        | Weight                               |
| `%mat.units%`      | Unit of measurement                 |