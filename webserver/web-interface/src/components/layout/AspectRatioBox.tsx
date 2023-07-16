import * as React from 'react';
import { Box, BoxProps } from '@mui/material';

interface AspectRatioBoxProps extends BoxProps {
  children: React.ReactNode;
  ratio?: number;
}

const AspectRatioBox = React.forwardRef(({
  children, ratio = 1, sx,
}: AspectRatioBoxProps, ref) => (
  <Box sx={{ position: 'relative' }}>
    <Box
      ref={ref}
      sx={{
        position: 'absolute',
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        '& > *': { height: '100%', width: '100%' },
        ...sx,
      }}
    >
      {children}
    </Box>
    <div style={{ paddingBottom: `${(1 / ratio) * 100}%` }} />
  </Box>
));

// This is required somehow still by linter even though a default argument is passed.
// Seems like a known issue with forwardRef. https://github.com/jsx-eslint/eslint-plugin-react/issues/2856
AspectRatioBox.defaultProps = { ratio: 1 };

export default AspectRatioBox;
