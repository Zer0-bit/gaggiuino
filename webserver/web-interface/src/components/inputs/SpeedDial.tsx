import * as React from 'react';
import { styled } from '@mui/material/styles';
import Box from '@mui/material/Box';
import SpeedDial from '@mui/material/SpeedDial';
import MenuOpenIcon from '@mui/icons-material/MenuOpenOutlined';
import SpeedDialAction from '@mui/material/SpeedDialAction';
import ShareIcon from '@mui/icons-material/Share';
import SaveIcon from '@mui/icons-material/Save';
import DeleteIcon from '@mui/icons-material/Delete';
import CloudQueueIcon from '@mui/icons-material/CloudQueue';

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

export default function SpeedDialInput() {
  return (
    <Box sx={{ transform: 'translateZ(0px)', flexGrow: 1 }}>
      <Box sx={{ position: 'relative', mx : 2, my: 0 }}>
        <StyledSpeedDial
          ariaLabel='speed-dial'
          icon={<MenuOpenIcon />}
          direction='left'
        >
          {actions.map((action) => (
            <SpeedDialAction
              key={action.name}
              icon={action.icon}
              tooltipTitle={action.name}
            />
          ))}
        </StyledSpeedDial>
      </Box>
    </Box>
  );
}
