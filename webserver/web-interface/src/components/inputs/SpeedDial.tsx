/* eslint-disable no-console */
import React, { useCallback } from 'react';
import { styled } from '@mui/material/styles';
import Box from '@mui/material/Box';
import SpeedDial from '@mui/material/SpeedDial';
import MenuOpenIcon from '@mui/icons-material/MenuOpenOutlined';
import SpeedDialAction from '@mui/material/SpeedDialAction';
import ShareIcon from '@mui/icons-material/Share';
import SaveIcon from '@mui/icons-material/Save';
import DeleteIcon from '@mui/icons-material/Delete';
import CloudQueueIcon from '@mui/icons-material/CloudQueue';
import useShotDataStore from '../../state/ShotDataStore';
import { Shot } from '../../models/models';

const StyledSpeedDial = styled(SpeedDial)(({ theme }) => ({
  position: 'absolute',
  '&.MuiSpeedDial-directionUp, &.MuiSpeedDial-directionLeft': {
    bottom: theme.spacing(2),
    right: theme.spacing(2),
  },
  '&.MuiSpeedDial-directionDown, &.MuiSpeedDial-directionRight': {
    top: theme.spacing(2),
    left: theme.spacing(2),
  },
}));

const actions = [
  { icon: <DeleteIcon />, name: 'Delete' },
  { icon: <SaveIcon />, name: 'Save' },
  { icon: <CloudQueueIcon />, name: 'CloudUpload' },
  { icon: <ShareIcon />, name: 'Share' },
];

export default function SpeedDialInput({ shot }: {shot: Shot}) {
  const { addShotToHistory, removeShotFromHistory } = useShotDataStore();

  const handleClick = useCallback((actionName: string) => {
    if (actionName === 'Save') {
      addShotToHistory(shot);
      console.log(`Clicked on ${actionName}`);
    } else if (actionName === 'Delete') {
      removeShotFromHistory(shot);
      console.log(`Clicked on ${actionName}`);
    } else if (actionName === 'CloudUpload') {
      // TO-DO: Upload to some cloud provider maybe (Visualiser or maybe another one is avail)
      console.log(`Clicked on ${actionName}`);
    } else if (actionName === 'Share') {
      // TO-DO: Share a profile with the community aka profile export
      console.log(`Clicked on ${actionName}`);
    }
  }, [addShotToHistory, removeShotFromHistory, shot]);

  return (
    <Box sx={{ transform: 'translateZ(0px)', flexGrow: 1 }}>
      <Box sx={{ position: 'relative', mx: 2, my: 0 }}>
        <StyledSpeedDial ariaLabel="speed-dial" icon={<MenuOpenIcon />} direction="left">
          {actions.map((action) => (
            <SpeedDialAction
              key={action.name}
              icon={action.icon}
              tooltipTitle={action.name}
              onClick={() => handleClick(action.name)}
            />
          ))}
        </StyledSpeedDial>
      </Box>
    </Box>
  );
}
