/* eslint-disable no-console */
import React, { useMemo } from 'react';
import SpeedDial from '@mui/material/SpeedDial';
import MenuOpenIcon from '@mui/icons-material/MenuOpenOutlined';
import SpeedDialAction from '@mui/material/SpeedDialAction';
import ShareIcon from '@mui/icons-material/Share';
import SaveIcon from '@mui/icons-material/Save';
import DeleteIcon from '@mui/icons-material/Delete';
import CloudQueueIcon from '@mui/icons-material/CloudQueue';
import { Shot } from '../../models/models';
import useShotDataStore from '../../state/ShotDataStore';

export interface ShotSpeedDialProps {
  shot: Shot;
  onSave: (shot: Shot) => void;
  onDelete: (shot: Shot) => void;
}

export default function ShotSpeedDial({ shot, onSave, onDelete }: ShotSpeedDialProps) {
  const { shotHistory } = useShotDataStore();
  const shotAlreadyInHistory = useMemo(() => shotHistory.find((s) => s.time === shot.time), [shot, shotHistory]);

  return (
    <SpeedDial ariaLabel="speed-dial" icon={<MenuOpenIcon />} direction="left">
      {shotAlreadyInHistory && <SpeedDialAction icon={<DeleteIcon />} tooltipTitle="Delete" onClick={() => onDelete(shot)} />}
      {!shotAlreadyInHistory && <SpeedDialAction icon={<SaveIcon />} tooltipTitle="Save" onClick={() => onSave(shot)} />}
      <SpeedDialAction icon={<CloudQueueIcon />} tooltipTitle="Cloud Upload" />
      <SpeedDialAction icon={<ShareIcon />} tooltipTitle="Share" />
    </SpeedDial>
  );
}
