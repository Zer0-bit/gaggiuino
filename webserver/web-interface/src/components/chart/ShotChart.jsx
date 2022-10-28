import React, {
  useEffect, useRef, useState, useMemo,
} from 'react';
import PropTypes from 'prop-types';
import { Line } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  TimeScale,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';
import { useTheme } from '@mui/material';
import getShotChartConfig from './ChartConfig';

ChartJS.register(
  CategoryScale,
  LinearScale,
  TimeScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
);

function getLabels(input) {
  return input.map((dp) => {
    const millisecons = dp.timeInShot % 1000;
    const seconds = Math.floor(dp.timeInShot / 1000);
    const minutes = Math.floor(seconds / 60);
    return `${minutes}:${seconds % 60}:${millisecons}`;
  });
}

function getDataset(input, key) {
  return input.map((dp) => dp[key]);
}

function mapToChartData(input, theme) {
  return {
    labels: getLabels(input),
    datasets: [
      {
        label: 'Temperature',
        data: getDataset(input, 'temp'),
        backgroundColor: theme.palette.temperature.main,
        borderColor: theme.palette.temperature.main,
        tension: 0.3,
        yAxisID: 'y1',
      },
      {
        label: 'Pressure',
        data: getDataset(input, 'pressure'),
        backgroundColor: theme.palette.pressure.main,
        borderColor: theme.palette.pressure.main,
        tension: 0.3,
        yAxisID: 'y2',
      },
      {
        label: 'Flow',
        data: getDataset(input, 'flow'),
        backgroundColor: theme.palette.flow.main,
        borderColor: theme.palette.flow.main,
        tension: 0.3,
        yAxisID: 'y2',
      },
      {
        label: 'Weight',
        data: getDataset(input, 'weight'),
        backgroundColor: theme.palette.weight.main,
        borderColor: theme.palette.weight.main,
        tension: 0.3,
        yAxisID: 'y1',
      },
    ],
  };
}

function Chart({ data }) {
  const chartRef = useRef(null);
  const theme = useTheme();
  const config = useMemo(() => getShotChartConfig(theme), [theme]);
  const [chartData, setChartData] = useState(mapToChartData([], theme));

  useEffect(() => {
    setChartData(mapToChartData(data, theme));
  }, [data]);

  return (
    <Line
      ref={chartRef}
      options={config}
      data={chartData}
    />
  );
}

export default Chart;

Chart.propTypes = {
  data: PropTypes.arrayOf(PropTypes.shape({
    timeInShot: PropTypes.number,
    temp: PropTypes.number,
    pressure: PropTypes.number,
  })).isRequired,
};
