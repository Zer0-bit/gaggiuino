import * as React from 'react';
import Box from '@mui/material/Box';
import Button from '@mui/material/Button';
import Typography from '@mui/material/Typography';
import Modal from '@mui/material/Modal';

const style = {
    position: 'absolute' as 'absolute',
    top: '50%',
    left: '50%',
    transform: 'translate(-50%, -50%)',
    width: '50%',
    bgcolor: 'background.paper',
    border: '10px solid',
    boxShadow: 50,
    p: 10,
    justifyContent: 'center'
};

export interface BasicModalProps {
  text: string;
  level: string;
}

export default function ShowAlert({ level, text }: BasicModalProps) {
    const [open, setOpen] = React.useState(false);
    const handleOpen = () => setOpen(true);
    const handleClose = () => setOpen(false);

    React.useEffect(() => {
        if (level && text) {
            handleOpen();
            const timer = setTimeout(handleClose, 3000); // Auto-close after 3 seconds
            return () => clearTimeout(timer); // Cleanup the timer on component unmount
        }
    }, [level, text]);

    const getBorderColor = () => {
        // Define your border colors based on the level value
        if (level === 'INFO') {
          return '#178bca';
        }
        if (level === 'WARN') {
          return 'orange';
        }
        if (level === 'ERROR') {
          return '#FF5733';
        }
        // Default border color if level doesn't match any conditions
        return 'gray';
      };
    
    const dynamicStyle = {
        ...style,
        border: `10px solid ${getBorderColor()}`, // Set the border color dynamically
    };

    return (
        <div>
            {/* <Button onClick={handleOpen}>Alert</Button> */}
            <Modal
                open={open}
                onClose={handleClose}
                aria-labelledby="modal-alert-type"
                aria-describedby="modal-alert-description"
            >
                <Box sx={dynamicStyle}>
                <Typography id="modal-alert-type" variant="h3" component="h3">
                    {level}
                </Typography>
                <Typography id="modal-alert-description" variant="h5" sx={{ mt: 2 }}>
                    {text}
                </Typography>
                </Box>
            </Modal>
        </div>
    );
}
