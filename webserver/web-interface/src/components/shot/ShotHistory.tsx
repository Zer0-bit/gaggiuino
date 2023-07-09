import Brightness3Icon from '@mui/icons-material/Brightness3';
import Brightness5Icon from '@mui/icons-material/Brightness5';
import Brightness7Icon from '@mui/icons-material/Brightness7';
import NightsStayIcon from '@mui/icons-material/NightsStay';
import {
  Avatar,
  ButtonBase,
  List, ListItem, ListItemAvatar,
  ListItemText,
} from '@mui/material';
import React, { useCallback, useState } from 'react';
import { formatTime } from '../../models/api';
import useShotDataStore from '../../state/ShotDataStore';
import ShotDialog from '../../pages/home/ShotDialog';
import { Shot } from '../../models/models';

enum TimeOfDay {
  Morning = 'Morning',
  Noon = 'Noon',
  Afternoon = 'Afternoon',
  Evening = 'Evening',
}

const getTimeOfDay = (date: Date): TimeOfDay => {
  const hours = date.getHours();
  if (hours < 12) {
    return TimeOfDay.Morning;
  } if (hours < 14) {
    return TimeOfDay.Noon;
  } if (hours < 18) {
    return TimeOfDay.Afternoon;
  }
  return TimeOfDay.Evening;
};

const getIcon = (timeOfDay: TimeOfDay) => {
  switch (timeOfDay) {
    case TimeOfDay.Morning:
      return <Brightness7Icon />;
    case TimeOfDay.Noon:
      return <Brightness5Icon />;
    case TimeOfDay.Afternoon:
      return <Brightness3Icon />;
    case TimeOfDay.Evening:
      return <NightsStayIcon />;
    default:
      return <Brightness7Icon />;
  }
};

const getColorForTimeOfDay = (timeOfDay: TimeOfDay): string => {
  switch (timeOfDay) {
    case TimeOfDay.Morning:
      return 'yellow';
    case TimeOfDay.Noon:
      return 'orange';
    case TimeOfDay.Afternoon:
      return 'blue';
    case TimeOfDay.Evening:
      return 'purple';
    default:
      return 'gray';
  }
};

export default function ShotHistory() {
  const { shotHistory } = useShotDataStore();
  const [viewingShot, setViewingShot] = useState<Shot | undefined>(undefined);

  const handleShotSelected = useCallback((shot: Shot) => {
    console.log(shot);
    setViewingShot(shot);
  }, [setViewingShot]);

  const handleSetOpen = useCallback((state: boolean) => {
    if (!state) {
      setViewingShot(undefined);
    }
  }, [setViewingShot]);

  return (
    <>
      <List>
        {shotHistory.sort((s1, s2) => s2.time - s1.time).map((shot) => {
          const shotDate = new Date(shot.time);
          const timeOfDay = getTimeOfDay(shotDate);
          const icon = getIcon(timeOfDay);
          const color = getColorForTimeOfDay(timeOfDay);
          const duration = formatTime({ time: shot.datapoints[shot.datapoints.length - 1].timeInShot });
          const formattedTimeInDate = shotDate.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });

          return (
            <ListItem key={shot.time} disableGutters>
              <ButtonBase component="div" style={{ width: '100%' }} onClick={() => handleShotSelected(shot)}>
                <ListItemAvatar>
                  <Avatar sx={{ color }}>
                    {icon}
                  </Avatar>
                </ListItemAvatar>
                <ListItemText primary={`Shot at ${formattedTimeInDate}.`} secondary={`Duration ${duration}`} />
              </ButtonBase>
            </ListItem>
          );
        })}
      </List>
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
