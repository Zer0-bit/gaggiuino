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
import { Shot } from '../../models/models';

// const StyledSpeedDial = styled(SpeedDial)(({ theme }) => ({
//   position: 'absolute',
//   '&.MuiSpeedDial-directionUp, &.MuiSpeedDial-directionLeft': {
//     bottom: theme.spacing(2),
//     right: theme.spacing(2),
//   },
//   '&.MuiSpeedDial-directionDown, &.MuiSpeedDial-directionRight': {
//     top: theme.spacing(2),
//     left: theme.spacing(2),
//   },
// }));

const actions = [
  { icon: <DeleteIcon />, name: 'Delete' },
  { icon: <SaveIcon />, name: 'Save' },
  { icon: <CloudQueueIcon />, name: 'CloudUpload' },
  { icon: <ShareIcon />, name: 'Share' },
];

export interface ShotSpeedDialProps {
  shot: Shot;
  onSave: (shot: Shot) => void;
  onDelete: (shot: Shot) => void;
}

export default function ShotSpeedDial({ shot, onSave, onDelete }: ShotSpeedDialProps) {
  const handleClick = useCallback((actionName: string) => {
    console.log(`Clicked on ${actionName}`);
    if (actionName === 'Save') {
      onSave(shot);
    } else if (actionName === 'Delete') {
      onDelete(shot);
    } else if (actionName === 'CloudUpload') {
      // TO-DO: Upload to some cloud provider maybe (Visualiser or maybe another one is avail)
    } else if (actionName === 'Share') {
      // TO-DO: Share a profile with the community aka profile export
    }
  }, [onSave, onDelete, shot]);

  return (
    <SpeedDial ariaLabel="speed-dial" icon={<MenuOpenIcon />} direction="left">
      {actions.map((action) => (
        <SpeedDialAction
          key={action.name}
          icon={action.icon}
          tooltipTitle={action.name}
          onClick={() => handleClick(action.name)}
        />
      ))}
    </SpeedDial>
  );
}
