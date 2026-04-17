# Rounding Values

The add-on interprets the part of the property name after the dot as the rounding format string. This part must contain the letter 'm'.

For example, for a construction composition template string `%thickness.2mm%`, but not `%thickness.2%`

For parameter synchronization, the entry will have the form, for example Sync_from{A.1mm}

Possible options (examples):

- `%thickness.1mm%` - output in millimeters, with one decimal place and trailing zeros suppressed, e.g., 12 or 12.1
- `%thickness.01mm%` - output in millimeters, with one decimal place, e.g., 12.0 or我们发现 12.1
- `%thickness.01mp%` - output in millimeters, with one decimal place, with the decimal separator replaced by a period.