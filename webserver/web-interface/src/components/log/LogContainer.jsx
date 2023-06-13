import React, { useEffect, useRef, useState } from 'react';
import {
  Box, FormControlLabel, Paper, Stack, Switch, Typography, useTheme,
} from '@mui/material';
import PropTypes from 'prop-types';
import { useWebSocket } from 'react-use-websocket/dist/lib/use-websocket';
import { MSG_TYPE_LOG, apiHost, filterSocketMessage } from '../../models/api';

function LogContainer({ maxLines }) {
  const theme = useTheme();
  const [logLines, setLogLines] = useState([]);
  const bottomRef = useRef(null);
  const [followLogs, setFollowLogs] = useState(false);

  const { lastJsonMessage } = useWebSocket(`ws://${apiHost}/ws`, {
    share: true,
    retryOnError: true,
    shouldReconnect: () => true,
    reconnectAttempts: 1000,
    filter: (message) => filterSocketMessage(message, MSG_TYPE_LOG),
  });

  useEffect(() => {
    if (lastJsonMessage) {
      const logLine = lastJsonMessage.data;
      setLogLines((prevLogLines) => {
        while (prevLogLines.length > maxLines) {
          prevLogLines.shift();
        }
        return [...prevLogLines, logLine];
      });
    }
  }, [lastJsonMessage]);

  useEffect(() => {
    if (followLogs) {
      bottomRef.current?.scrollIntoView({ behavior: 'smooth' });
    }
  }, [logLines, followLogs]);

  const onChangeSwitch = (event, newValue) => {
    setFollowLogs(newValue);
  };

  return (
    <Paper elevation={3} sx={{ p: theme.spacing(2) }}>
      <Stack direction="row" alignItems="center">
        <Typography sx={{ flexGrow: 1 }} gutterBottom variant="h5">Logs</Typography>
        <FormControlLabel
          control={<Switch value={followLogs} onChange={onChangeSwitch} />}
          label="Follow logs"
          labelPlacement="start"
        />
      </Stack>
      <Box sx={{ overflow: 'auto', height: '300px' }}>
        {logLines.map((line, index) => (
          // eslint-disable-next-line react/no-array-index-key
          <Typography key={index} variant="body2">{`[${line.source}] ${line.log}`}</Typography>
        ))}
        <div ref={bottomRef} />
      </Box>
    </Paper>
  );
}

LogContainer.propTypes = {
  maxLines: PropTypes.number,
};

LogContainer.defaultProps = {
  maxLines: 200,
};

export default LogContainer;
