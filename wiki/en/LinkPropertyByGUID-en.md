## Synchronization of Hierarchical Structure Properties (Curtain Wall, Elements in Zone)

Similar to synchronization within a single element, with the addition of the _sub suffix, for example `Sync_from_sub{Property:GROUP_NAME/PROPERTY_NAME}`, written in the curtain wall will read the property from the nested element (panel, frame, accessory). Similarly, `Sync_to_sub{Property:GROUP_NAME/PROPERTY_NAME}` works.

## _v1.76_ Element Property Synchronization by GUID

Workflow:
* Select the child elements (the elements into which information needs to be written). The child elements must be editable.
* Run the **Link Elements** function
* Click on the parent element (whose properties will be written into the child elements)

`Sync_from_GUID{Property:GROUP_NAME/PROPERTY_NAME_C_GUID; Property:GROUP_NAME/PROPERTY_NAME}`

Add `Sync_GUID` to the description of the `Property:GROUP_NAME/PROPERTY_NAME_C_GUID` property.