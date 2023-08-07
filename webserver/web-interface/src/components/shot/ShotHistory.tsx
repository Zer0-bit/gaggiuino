import Brightness5Icon from '@mui/icons-material/Brightness5';
import Brightness7Icon from '@mui/icons-material/Brightness7';
import NightsStayIcon from '@mui/icons-material/NightsStay';
import {
  Avatar,
  ButtonBase,
  List, ListItem, ListItemAvatar,
  ListItemText,
  Typography,
  alpha,
} from '@mui/material';
import React, { useCallback, useEffect, useState } from 'react';
import formatTime from '../../models/time_format';
import useShotDataStore from '../../state/ShotDataStore';
import ShotDialog from '../../pages/ShotDialog';
import { Shot } from '../../models/models';

enum TimeOfDay {
  Morning = 'Morning',
  Afternoon = 'Afternoon',
  Evening = 'Evening',
}

const getTimeOfDay = (date: Date): TimeOfDay => {
  const hours = date.getHours();
  if (hours > 5 && hours < 12) {
    return TimeOfDay.Morning;
  } if (hours < 18) {
    return TimeOfDay.Afternoon;
  }
  return TimeOfDay.Evening;
};

const getIcon = (timeOfDay: TimeOfDay) => {
  switch (timeOfDay) {
    case TimeOfDay.Morning:
      return <Brightness7Icon />;
    case TimeOfDay.Afternoon:
      return <Brightness5Icon />;
    case TimeOfDay.Evening:
      return <NightsStayIcon />;
    default:
      return <Brightness7Icon />;
  }
};

const getColorForTimeOfDay = (timeOfDay: TimeOfDay): string => {
  switch (timeOfDay) {
    case TimeOfDay.Morning:
      return '#ffea00';
    case TimeOfDay.Afternoon:
      return '#ffa733';
    case TimeOfDay.Evening:
      return '#3d5afe';
    default:
      return '#444';
  }
};

export default function ShotHistory() {
  const shotHistory = useShotDataStore((state) => state.shotHistory);
  const [viewingShot, setViewingShot] = useState<Shot | undefined>(undefined);
  const [sortedShotHistory, setSortedShotHistory] = useState<Shot[]>([]);

  const handleShotSelected = useCallback((shot: Shot) => {
    setViewingShot(shot);
  }, [setViewingShot]);

  const handleSetOpen = useCallback((state: boolean) => {
    if (!state) {
      setViewingShot(undefined);
    }
  }, [setViewingShot]);

  useEffect(() => {
    const shallowCopy = [...shotHistory]; // avoid mutating state
    shallowCopy.sort((s1, s2) => s2.time - s1.time);
    setSortedShotHistory(shallowCopy);
  }, [setSortedShotHistory, shotHistory]);

  return (
    <>
      {shotHistory.length === 0 && <Typography variant="body2">Pull some shots to see them here.</Typography>}
      {shotHistory.length > 0 && (
      <List>
        {sortedShotHistory.map((shot) => {
          const shotDate = new Date(shot.time);
          const timeOfDay = getTimeOfDay(shotDate);
          const icon = getIcon(timeOfDay);
          const color = getColorForTimeOfDay(timeOfDay);
          const duration = formatTime({ time: shot.datapoints[shot.datapoints.length - 1].timeInShot });
          const formattedTimeInDay = shotDate.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
          const formattedDay = shotDate.toLocaleDateString([], { day: 'numeric', month: 'short' });
          // const theme = useTheme();

          return (
            <ListItem key={shot.time} disableGutters>
              <ButtonBase component="div" style={{ width: '100%' }} onClick={() => handleShotSelected(shot)}>
                <ListItemAvatar>
                  <Avatar sx={{ color, backgroundColor: alpha(color, 0.15) }}>
                    {icon}
                  </Avatar>
                </ListItemAvatar>
                <ListItemText primary={`${formattedDay}, ${formattedTimeInDay}.`} secondary={`Duration ${duration}`} />
              </ButtonBase>
            </ListItem>
          );
        })}
      </List>
      )}
      {viewingShot && (
      <ShotDialog
        open={!!viewingShot}
        setOpen={handleSetOpen}
        historyShot={viewingShot}
      />
      )}
    </>
  );
}
