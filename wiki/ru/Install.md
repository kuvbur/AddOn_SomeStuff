# WIN

1. In the [Releases](https://github.com/kuvbur/AddOn_SomeStuff/releases) section, download the latest version of the add-on for your Archicad version with the suffix WIN.  
2. If Archicad is running, close it.  
3. Copy the downloaded file to the Archicad Add-Ons folder (for example, _C:\Program Files\GRAPHISOFT\ARCHICAD 24\Add-Ons_).  
4. Launch Archicad.  

# MAC

1. In the [Releases](https://github.com/kuvbur/AddOn_SomeStuff/releases) section, download the latest version of the add-on for your Archicad version with the suffix MAC.  
2. After downloading and extracting the archive:  
   - Open Terminal  
   - Navigate to the add-on folder:  
   - Remove quarantine attributes:  
     ```
     xattr -cr SomeStuff.bundle
     ```  
     *(Use `sudo xattr -cr SomeStuff.bundle` if necessary)*  
   - **Sign the add-on** (mandatory for recent macOS versions):  
     ```
     codesign --force --deep --sign - SomeStuff.bundle
     ```  
3. Launch Archicad.  
4. In the Add-Ons Manager, click Add and point to the add-on.  

Example:  
<img src="https://github.com/user-attachments/assets/8d162855-9b67-4f8e-a1aa-22b3fc2a5725" />

# Adding add-on commands to the menu / Command Palette

1. Open the Options -> Work Environment menu.  
2. Go to the Menus or Toolbars section.  
3. On the right side of the window, create a new menu or command palette or open an existing one.  
4. On the left side, select All commands by theme, then go to Add-Ons -> SomeStuff.  
5. Add the desired commands.  
<img width="953" height="1018" alt="изображение" src="https://github.com/user-attachments/assets/994696fa-8b3c-48fa-af74-5f2bf9a52629" />
