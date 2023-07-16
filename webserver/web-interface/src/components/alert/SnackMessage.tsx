import {
  Alert, AlertColor, Snackbar, SnackbarCloseReason,
} from '@mui/material';
import React, { ReactNode, useEffect, useState } from 'react';

export interface SnackMessage {
  content: ReactNode;
  level: AlertColor;
}

interface SnackNotificationProps {
  message?: SnackMessage;
}

function SnackNotification({ message = undefined }: SnackNotificationProps) {
  const [messageInternal, setMessageInternal] = useState<SnackMessage | undefined>();
  const [key, setKey] = useState(0);

  useEffect(() => {
    setKey((prevKey) => prevKey + 1);
    setMessageInternal(message);
  }, [message]);

  const handleClose = (event: React.SyntheticEvent | Event, reason?: SnackbarCloseReason) => {
    if (reason === 'clickaway') {
      return;
    }

    setMessageInternal(undefined);
  };

  return (
    <Snackbar
      key={key}
      open={messageInternal !== undefined}
      autoHideDuration={3000}
      anchorOrigin={{ vertical: 'bottom', horizontal: 'center' }}
      onClose={handleClose}
    >
      <Alert onClose={handleClose} severity={message?.level} sx={{ width: '100%' }}>
        {message?.content}
      </Alert>
    </Snackbar>

  );
}

export default SnackNotification;
