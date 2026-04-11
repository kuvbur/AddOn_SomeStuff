# Filling Change Data According to GOST 21.1101

The function is only compatible with the change marker and title block from the [kuvbur_Format by GOST](https://github.com/kuvbur/gdl_bibl/tree/master/kuvbur_%D0%A4%D0%BE%D1%80%D0%BC%D0%B0%D1%82%20%D0%BF%D0%BE%20%D0%93%D0%9E%D0%A1%D0%A2) library. It is designed to fill title blocks and change markers in accordance with GOST R 21.101-2020.

The button is active only when called from a layout. The processing applies to all visible and reserved change markers located on layouts. For correct operation, it is also necessary to reserve the layout parameters.

When working collaboratively, caution must be exercised - **DO NOT RESERVE OTHER USERS' SHEETS**. **The function's actions cannot be undone.**

The add-on iterates through each sheet, collecting information about the quantity and parameters of change markers. For each sheet with changes, data is written to the layout properties `Somestuff_QtyIssue_1...n`. Notes with change numbers and types are written to the property `Somestuff_Note`. The add-on writes the section number into each change marker. To regenerate a sheet (if it did not happen automatically), it is sufficient to switch to another sheet.

After loading the properties, select all layouts and write spaces into the properties with the name `Somestuff_`.

An example file and schema for loading can be viewed [here](https://github.com/kuvbur/gdl_bibl/tree/master/kuvbur_%D0%A4%D0%BE%D1%80%D0%BC%D0%B0%D1%82%20%D0%BF%D0%BE%20%D0%93%D0%9E%D0%A1%D0%A2)