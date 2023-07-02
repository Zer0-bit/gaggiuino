import { Doughnut } from 'react-chartjs-2';
import PropTypes from 'prop-types';
import {
  Chart as ChartJS,
  ArcElement,
  Title,
} from 'chart.js';
import { useTheme } from '@mui/material';
import GaugeCentralTextPlugin from './GaugeCentralTextPlugin';

ChartJS.register(ArcElement, Title, GaugeCentralTextPlugin);

function GaugeChart({
  value,
  maxValue = 0,
  primaryColor,
  unit,
  title,
  maintainAspectRatio = false,
}) {
  const theme = useTheme();

  const options = {
    cutout: '90%',
    borderWidth: 0,
    responsive: true,
    updateMode: 'resize',
    maintainAspectRatio: {maintainAspectRatio},
    plugins: {
      center: {
        text: value.toFixed(1) + unit,
        color: primaryColor,
        maxFontSize: 50,
      },
      title: {
        display: title && title.length > 0,
        text: title,
      },
    },
  };

  const data = {
    cutout: '90%',
    datasets: [{
      data: [value, Math.max(0, maxValue - value)],
      backgroundColor: [
        primaryColor,
        theme.palette.divider,
      ],
    }],
  };
  return <Doughnut data={data} options={options} />;
}

export default GaugeChart;

GaugeChart.propTypes = {
  value: PropTypes.number.isRequired,
  maxValue: PropTypes.number,
  primaryColor: PropTypes.string,
  unit: PropTypes.string,
  title: PropTypes.string,
};

GaugeChart.defaultProps = {
  maxValue: 100,
  primaryColor: '#6296C5',
  unit: '',
  title: '',
};
