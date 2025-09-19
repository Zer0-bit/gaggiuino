## Creating a profile:

> [!WARNING]  
> Builds **before 2025-03-31** don't strip the **profile ID** from the json when exporting - the ID is system specific and might clash with other people's IDs. Before sharing, update to latest build (recommended) or manually strip the profile ID. 

The profile name must be the same in 3 locations so the profile is properly linked to the description (see image):
- `profile name`.json ( the name of the json file )
- "name": "`profile name`", ( the name used inside the json structure )
- `profile name`.md ( the name of the markdown file )

![image](https://github.com/user-attachments/assets/cce932b1-f4cd-45e5-afca-006e63dcc7e9)