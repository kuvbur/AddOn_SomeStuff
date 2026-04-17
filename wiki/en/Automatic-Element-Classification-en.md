## Purpose of Classification by Property

The add-on allows for automatic assignment and synchronization of element classification based on property values.

### Enabling Classification for Unclassified Elements

Automatic assignment of classification to elements without classification

Add the value to the class description: `some_stuff_class`  
*Example configuration:*  
![image](https://github.com/user-attachments/assets/b87efbe3-094c-4f9a-895d-57a7bf38cc2b)

### Change Class Based on Property Value

Dynamically change an element's class when a linked property is modified.

1. Create a text property.
2. In the property description, specify:  
   `Sync_to{Class:CLASSIFICATION_NAME}`  
   _(The class is searched for by its ID.)_
3. Add your own formula to the property, which will output a class ID as a result.

_Configuration Example:_  
![image](https://github.com/user-attachments/assets/2e1c08cb-1b36-4a4a-8cae-1f6a6da39f69)

### Outputting Class Information

Create a property with the description:  
`Sync_from{Class:CLASSIFICATION_NAME; FullName}`

- Outputs the full name of the element's class

## Disabling Classification Processing

**Options:**

- Using the main processing flag:  
  Property `Sync_flag` (completely disables element processing)
- Creating a specialized flag:  
  Property with description `Sync_class_flag` (disables only classification processing)

## Examples

- [Video example on YouTube](https://youtu.be/xTOY58xC4Tg)
- [Video example on Rutube](https://rutube.ru/video/87890fab67db3b1c2d2d7067a37e0475)