import {
  Avatar,
  ButtonBase,
  List, ListItem, ListItemAvatar,
  ListItemText,
  alpha,
  useTheme,
} from '@mui/material';
import React, { useCallback } from 'react';
import useProfileStore from '../../state/ProfileStore';

interface AvailableProfileSelectorProps {
  selectedProfileId?: number;
  onSelected?: (profileId: number) => void;
}

export default function AvailableProfileSelector({
  onSelected = undefined, selectedProfileId = undefined,
}: AvailableProfileSelectorProps) {
  const { availableProfiles } = useProfileStore();

  const handleProfileSelected = useCallback(async (profileId: number) => {
    onSelected && onSelected(profileId);
  }, [onSelected]);

  const theme = useTheme();
  return (
    <List>
      {availableProfiles.map((profileSummary) => (
        <ListItem
          key={profileSummary.id}
          disableGutters
          sx={{ p: 0 }}
        >
          <ButtonBase
            component="div"
            sx={{ width: '100%', p: 1, backgroundColor: profileSummary.id === selectedProfileId ? alpha(theme.palette.primary.main, 0.1) : 'transparent' }}
            onClick={() => handleProfileSelected(profileSummary.id)}
          >
            <ListItemAvatar>
              <Avatar>
                { profileSummary.name
                  .split(' ')
                  .map((word) => word.replace(/[^a-zA-Z0-9]/g, '')[0]) // Ignore everything that's not a letter or a number
                  .join('') // Initials
                  .substring(0, 2) // Keep 2
                  .toLocaleUpperCase() }
              </Avatar>
            </ListItemAvatar>
            <ListItemText primary={profileSummary.name} />
          </ButtonBase>
        </ListItem>
      ))}
    </List>
  );
}
