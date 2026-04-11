## Element Numbering

[Example file for AC25](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/files/%D0%9D%D1%83%D0%BC%D0%B5%D1%80%D0%B0%D1%86%D0%B8%D1%8F_25.pln)

Elements with the same value for the criterion property are assigned the same numbers (positions).

For numbering to work, two properties must be created:

- A flag property to enable numbering
- A property to store the position (number)

Numbering rules and flags are specified in the element's property descriptions.

_v1.7+_ When numbering, the positions of selected locked elements (including those within a module) are considered.

_v1.74+_ If only one element is selected, all elements from visible layers that have the numbering rules of the selected element available will be processed.

**Numbering enable flag in one of the formats**
| Flag Format | Description | Version |
|----------------------------------------|--------------------------------------------------------|---------|
| `Renum_flag{property_name}` | Standard numbering | |
| `Renum_flag{property_name ; NULL}` | Automatic zero-padding considering groups (e.g., `1` → `001`) | |
| `Renum_flag{property_name ; SPACE}` | Automatic space-padding considering groups (e.g., `1` → `  1`) | v1.6 |
| `Renum_flag{property_name ; ALLNULL}` | Automatic zero-padding without considering groups | |
| `Renum_flag{property_name ; ALLSPACE}` | Automatic space-padding without considering groups | |
| `Renum_flag{property_name ; n_NULL}` | Fixed zero-padding (up to `n` characters) considering groups | v1.76 |
| `Renum_flag{property_name ; n_SPACE}` | Fixed space-padding (up to `n` characters) considering groups | v1.76 |
| `Renum_flag{property_name ; n_ALLNULL}` | Fixed zero-padding (up to `n` characters) without considering groups | v1.76 |
| `Renum_flag{property_name ; n_ALLSPACE}`| Fixed space-padding (up to `n` characters) without considering groups | v1.76 |

**Operation specifics:**

1. Automatic length determination:
   - For `NULL/SPACE/ALLNULL/ALLSPACE`, the padding length is determined by the maximum position in the group
   - Example: positions `[5, 12, 100]` → number of characters in the maximum position is 3 → padding length → `005`, `012`, `100`

2. Fixed length specification (n\_* commands):
   - `n` - minimum length of the resulting string
   - If the maximum position is longer than `n` - the actual length is used
   - Example: `4_NULL` for positions `[1, 25, 100]` → `0001`, `0025`, `0100`

`Property name with the rule` is specified in the format Property:_group name_/_property name_
Data type of the flag property: Criterion (TRUE/FALSE), _v1.6_ or a set of parameters with values Add, Renumber, Exclude

![Example 1](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/renum_flag_1-ru.PNG)
![Example 2](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/renum_flag_allnull-ru.PNG)

**Position property in one of the formats**
| Format | Description | Version |
|-----------------------------------------|--------------------------------------------------------------------------|--------|
| `Renum{criterion_property_name}` | Standard numbering by one criterion | |
| `Renum{criterion_property_name; split_property_name}` | Numbering with grouping by an additional property | |
| `Renum{library_parameter_criterion}` | Using a library element parameter instead of a criterion property | v1.6 |
| `Renum{library_parameter_criterion; library_parameter_split}` | Using library element parameters for both criteria | v1.6 |

**Clarifications:**

1. **Criterion property** - defines the group of elements for joint numbering
2. **Split property** - separates one numbering group from another. Each group has independent numbering.
3. Library parameters must be accessible via the Archicad API

![Example 3](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/renum_pos-ru.PNG)
![Example 4](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/renum_pos_delim-ru.PNG)

To write the resulting position to the ID, add `Sync_to{ID}` to the description of the position property.

![Example 5](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/renum_pos_id-ru.PNG)

## Possible Errors

| Error Message | Cause | Solution |
| ------------- | ----- | -------- |
| Properties specified in numbering rules not found | | |
| Some elements were not numbered due to errors in properties | | |
| Numbering change is not required | | |
| Error in numbering rule | | |