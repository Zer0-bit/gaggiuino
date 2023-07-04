import React, { useEffect, useRef, useState } from 'react';
import {
  Box, FormControlLabel, Paper, Stack, Switch, Typography, useTheme,
} from '@mui/material';
import useLogMessageStore from '../../state/LogStore';

function LogContainer() {
  const theme = useTheme();
  const bottomRef = useRef(null);
  const [followLogs, setFollowLogs] = useState(false);

  const { logs } = useLogMessageStore();

  useEffect(() => {
    if (followLogs) {
      bottomRef.current?.scrollIntoView({ behavior: 'smooth' });
    }
  }, [logs, followLogs]);

  const onChangeSwitch = (event, newValue) => {
    setFollowLogs(newValue);
  };

  return (
    <Paper elevation={3} sx={{ p: theme.spacing(2), width: '100%' }}>
      <Stack direction="row" alignItems="center" sx={{ height: '100%', width: '100%' }}>
        <Typography sx={{ flexGrow: 1 }} gutterBottom variant="h5">Logs</Typography>
        <FormControlLabel
          control={<Switch value={followLogs} onChange={onChangeSwitch} />}
          label="Follow logs"
          labelPlacement="start"
        />
      </Stack>
      <Box sx={{ overflow: 'auto', height: '300px', width: '100%' }}>
        {logs.map((line, index) => (
          // eslint-disable-next-line react/no-array-index-key
          <Typography key={index} variant="body2">{`[${line.source}] ${line.log}`}</Typography>
        ))}
        <div ref={bottomRef} />
      </Box>
    </Paper>
  );
}

export default LogContainer;
