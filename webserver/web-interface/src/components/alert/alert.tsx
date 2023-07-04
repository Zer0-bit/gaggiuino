import * as React from 'react';
import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import Modal from '@mui/material/Modal';

const style = {
    position: 'absolute' as 'absolute',
    top: '0%',
    left: '70%',
    transform: 'translate(-0%, -0%)',
    width: '30%',
    bgcolor: 'background.paper',
    border: '2px solid',
    boxShadow: 50,
    p: 1,
    justifyContent: 'center',
    borderRadius: '16px'
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
        border: `2px solid ${getBorderColor()}`, // Set the border color dynamically
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
                <Typography id="modal-alert-type" variant="h6" component="h6">
                    {level}
                </Typography>
                <Typography id="modal-alert-description" variant="h6" sx={{ mt: 2 }}>
                    {text}
                </Typography>
                </Box>
            </Modal>
        </div>
    );
}
