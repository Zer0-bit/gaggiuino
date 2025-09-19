## Creating a profile:

The profile name must be the same in 3 locations so the profile is properly linked to the description (see image):
- `profile name`.json ( the name of the json file )
- "name": "`profile name`", ( the name used inside the json structure )
- `profile name`.md ( the name of the markdown file )

![image](https://github.com/user-attachments/assets/cce932b1-f4cd-45e5-afca-006e63dcc7e9)

> [!WARNING|style:callout|label:Profile ID|iconVisibility:visible]
> If exporting a profile from a build **before 2025-03-31** you'll need to strip the **profile ID** from the json - the ID is system specific and might clash with other people's IDs. This is done automatically on newer builds.