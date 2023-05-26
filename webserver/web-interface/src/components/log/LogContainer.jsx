import React, { useEffect, useRef, useState } from 'react';
import {
  Box, Paper, Typography, useTheme,
} from '@mui/material';
import PropTypes from 'prop-types';

function LogContainer({ logLine }) {
  const theme = useTheme();
  const [logLines, setLogLines] = useState([]);
  const bottomRef = useRef(null);

  useEffect(() => {
    if (logLine) {
      setLogLines((prevLogLines) => {
        while (prevLogLines.length > 400) {
          prevLogLines.shift();
        }
        return [...prevLogLines, logLine];
      });
    }
  }, [logLine]);

  useEffect(() => {
    bottomRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [logLines]);

  return (
    <Paper elevation={3} sx={{ p: theme.spacing(2) }}>
      <Typography gutterBottom variant="h5">Logs</Typography>

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
  logLine: PropTypes.shape({
    source: PropTypes.string.isRequired,
    log: PropTypes.string.isRequired,
  }),
};

LogContainer.defaultProps = {
  logLine: undefined,
};

export default LogContainer;
