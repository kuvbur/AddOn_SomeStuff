# WIN

1. In the [Releases](https://github.com/kuvbur/AddOn_SomeStuff/releases) section, download the latest version of the add-on for the required version with the WIN suffix.
2. If ArchiCAD is running, close it.
3. Copy the downloaded file to the ArchiCAD extensions folder (e.g., _C:\Program Files\GRAPHISOFT\ARCHICAD 24\Расширения ARCHICAD\Дополнения_).
4. Launch ArchiCAD.

# MAC

1. In the [Releases](https://github.com/kuvbur/AddOn_SomeStuff/releases) section, download the latest version of the add-on for your required version with the MAC suffix.
2. After downloading and extracting the archive:
   - Open Terminal
   - Navigate to the folder containing the add-on
   - Remove quarantine attributes:
     ```bash
     xattr -cr SomeStuff.bundle
     ```
     _(Use `sudo xattr -cr SomeStuff.bundle` if necessary)_
   - **Sign the add-on** (mandatory step for newer macOS versions):
     ```bash
     codesign --force --deep --sign - SomeStuff.bundle
     ```
3. Launch ArchiCAD.
4. In the Add-On Manager, click Add and point to the add-on.

Example
<img src="https://github.com/user-attachments/assets/8d162855-9b67-4f8e-a1aa-22b3fc2a5725" />

# Adding Add-on Commands to Menu / Command Palette

1. Open the Options->Environment menu
2. Navigate to the Menu or Command Palette item
3. In the right part of the window, create a new menu or command palette, or open an existing one
4. In the left part, select All Commands by Topic, go to the list Extensions->SomeStuff
5. Add the required commands
   ![Adding Add-on Commands to Menu](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/add_menu-ru.PNG)