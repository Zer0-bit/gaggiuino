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
import { useTheme, alpha } from '@mui/material';
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

function mapDataPointToLabel(dataPoint) {
  if (!dataPoint.timeInShot) {
    return 0;
  }

  return dataPoint.timeInShot / 1000;
}

function getLabels(input) {
  return input.map(mapDataPointToLabel);
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
        data: getDataset(input, 'temperature'),
        backgroundColor: alpha(theme.palette.temperature.main, 0.8),
        borderColor: theme.palette.temperature.main,
        tension: 0.3,
        yAxisID: 'y1',
      },
      {
        label: 'Pressure',
        data: getDataset(input, 'pressure'),
        backgroundColor: alpha(theme.palette.pressure.main, 0.8),
        borderColor: theme.palette.pressure.main,
        tension: 0.3,
        yAxisID: 'y2',
      },
      {
        label: 'Pump Flow',
        data: getDataset(input, 'pumpFlow'),
        backgroundColor: alpha(theme.palette.flow.main, 0.8),
        borderColor: theme.palette.flow.main,
        tension: 0.3,
        yAxisID: 'y2',
      },
      {
        label: 'Weight Flow',
        data: getDataset(input, 'weightFlow'),
        backgroundColor: alpha(theme.palette.weightFlow.main, 0.8),
        borderColor: theme.palette.weightFlow.main,
        tension: 0.3,
        yAxisID: 'y2',
      },
      {
        label: 'Weight',
        data: getDataset(input, 'shotWeight'),
        backgroundColor: alpha(theme.palette.weight.main, 0.8),
        borderColor: theme.palette.weight.main,
        tension: 0.3,
        yAxisID: 'y1',
      },
      {
        label: 'Target Pressure',
        data: getDataset(input, 'targetPressure'),
        backgroundColor: alpha(theme.palette.pressure.main, 0.3),
        borderColor: alpha(theme.palette.pressure.main, 0.6),
        tension: 0.3,
        borderDash: [8, 4],
        yAxisID: 'y2',
      },
      {
        label: 'Target Flow',
        data: getDataset(input, 'targetPumpFlow'),
        backgroundColor: alpha(theme.palette.flow.main, 0.3),
        borderColor: alpha(theme.palette.flow.main, 0.6),
        tension: 0.3,
        borderDash: [8, 4],
        yAxisID: 'y2',
      },
    ],
  };
}

function popDataFromChartData(chartData) {
  chartData.labels.shift();
  chartData.datasets.forEach((dataset) => dataset.data.shift());
}

/**
 * Compares the timeInShot prop of the dataPoint to the last dataPoint in chartData.
 * If it's before it must mean a new shot was started.
 */
function newShotStarted(dataPoint, chartData) {
  const newTimeLabel = mapDataPointToLabel(dataPoint);
  const previousMaxTimeLabel = chartData.labels[chartData.labels.length - 1] || 0;
  return previousMaxTimeLabel > newTimeLabel;
}

function addDataPointToChartData(chartData, dataPoint, maxLength, chartRef) {
  while (!Number.isNaN(maxLength) && chartData.labels.length >= maxLength) {
    popDataFromChartData(chartData);
  }

  if (!dataPoint) {
    return;
  }

  chartData.labels.push(mapDataPointToLabel(dataPoint));
  chartData.datasets[0].data.push(dataPoint.temperature);
  chartData.datasets[1].data.push(dataPoint.pressure);
  chartData.datasets[2].data.push(dataPoint.pumpFlow);
  chartData.datasets[3].data.push(dataPoint.weightFlow);
  chartData.datasets[4].data.push(dataPoint.shotWeight);
  chartData.datasets[5].data.push(dataPoint.targetPressure);
  chartData.datasets[6].data.push(dataPoint.targetPumpFlow);

  /* eslint-disable no-param-reassign */
  chartRef.current.data.labels = chartData.labels;
  chartData.datasets.forEach((dataset, index) => {
    chartRef.current.data.datasets[index].data = dataset.data;
  });
  chartRef.current.update();
  /* eslint-enable no-param-reassign */
}

function Chart({ data, newDataPoint, maxLength }) {
  const chartRef = useRef(null);
  const theme = useTheme();
  const config = useMemo(() => getShotChartConfig(theme), [theme]);
  const [chartData, setChartData] = useState(mapToChartData([], theme));

  if (newDataPoint && data) {
    throw new Error("Only one of 'newDataPoint' or 'data' props must be defined");
  }

  // Refreshes the chart completely by reseting the chartData. This redraws the whole chart.
  useEffect(() => {
    if (data === undefined || data === null) {
      return;
    }
    setChartData(mapToChartData(data, theme));
  }, [data]);

  // Adds newDataPoint to the end of the chart unless it detects that a new shot was started. More efficient.
  useEffect(() => {
    if (!newDataPoint === undefined || newDataPoint === null) {
      return;
    }
    if (newShotStarted(newDataPoint, chartData)) {
      setChartData(mapToChartData([newDataPoint], theme));
    } else {
      addDataPointToChartData(chartData, newDataPoint, maxLength, chartRef);
    }
  }, [newDataPoint]);

  return (
    <Line
      ref={chartRef}
      options={config}
      data={chartData}
    />
  );
}

export default Chart;

export const ShotChartDataPointType = PropTypes.shape({
  timeInShot: PropTypes.number,
  temperature: PropTypes.number,
  pressure: PropTypes.number,
  pumpFlow: PropTypes.number,
  weightFlow: PropTypes.number,
  shotWeight: PropTypes.number,
  targetTemperature: PropTypes.number,
  targetPumpFlow: PropTypes.number,
  targetPressure: PropTypes.number,
});

Chart.propTypes = {
  data: PropTypes.arrayOf(ShotChartDataPointType),
  newDataPoint: ShotChartDataPointType,
  maxLength: PropTypes.number,
};

Chart.defaultProps = {
  data: undefined,
  newDataPoint: undefined,
  maxLength: undefined,
};
