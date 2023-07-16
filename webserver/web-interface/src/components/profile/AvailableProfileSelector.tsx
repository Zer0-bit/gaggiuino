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
        <ButtonBase component="div" style={{ width: '100%' }} onClick={() => handleProfileSelected(profileSummary.id)}>
          <ListItem
            key={profileSummary.id}
            disableGutters
            sx={{
              backgroundColor: profileSummary.id === selectedProfileId ? alpha(theme.palette.primary.main, 0.1) : 'transparent',
              p: 1,
            }}
          >
            <ListItemAvatar>
              <Avatar>
                { profileSummary.name
                  .split(' ').map((word) => word[0]).join('') // Initials
                  .substring(0, 2) // Keep 2
                  .toLocaleUpperCase() }
              </Avatar>
            </ListItemAvatar>
            <ListItemText primary={profileSummary.name} />
          </ListItem>
        </ButtonBase>
      ))}
    </List>
  );
}
