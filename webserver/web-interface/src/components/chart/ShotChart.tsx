import React, {
  useEffect, useRef, useState, useMemo, useCallback,
} from 'react';
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
  ChartData,
} from 'chart.js';
import { useTheme, alpha, Theme } from '@mui/material';
import { getShotChartConfig } from './ChartConfig';
import { ShotSnapshot } from '../../models/models';
import { isNewShotStarted } from '../../state/ShotDataStore';

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

function mapDataPointToLabel(dataPoint:ShotSnapshot) {
  return dataPoint.timeInShot;
}

function getLabels(input: ShotSnapshot[]) {
  return input.map(mapDataPointToLabel);
}

function getDataset(input:ShotSnapshot[], key: keyof ShotSnapshot) {
  return input.map((dp) => dp[key]);
}

function mapToChartData(input: ShotSnapshot[], theme: Theme) {
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

function popDataFromChartData(chartData: ChartData<'line'>) {
  chartData.labels?.shift();
  chartData.datasets.forEach((dataset) => dataset.data.shift());
}

/**
 * Compares the timeInShot prop of the dataPoint to the last dataPoint in chartData.
 * If it's before it must mean a new shot was started.
 */
function newShotStarted(newDataPoint:ShotSnapshot, existingDatapoints: ShotSnapshot[]) {
  return isNewShotStarted({ time: 0, datapoints: existingDatapoints }, newDataPoint);
}

// eslint-disable-next-line @typescript-eslint/no-explicit-any
function addDataPointToChartData(chartData: ChartData<'line'>, dataPoint: ShotSnapshot, maxLength: number | undefined, chartRef:React.MutableRefObject<any>) {
  while (maxLength && chartData.labels && chartData.labels.length >= maxLength) {
    popDataFromChartData(chartData);
  }

  if (!dataPoint) {
    return;
  }

  chartData.labels?.push(mapDataPointToLabel(dataPoint));
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

export interface ShotChartProps {
  data?: ShotSnapshot[],
  newDataPoint?: ShotSnapshot,
  maxLength?: number,
  onHover?: (datapoint: ShotSnapshot) => void,
}

function ShotChart({
  data = undefined,
  newDataPoint = undefined,
  maxLength = undefined,
  onHover = undefined,
}: ShotChartProps) {
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  const chartRef = useRef<any>();
  const theme = useTheme();
  const [datapoints, setDatapoints] = useState<ShotSnapshot[]>([]);

  const onHoverInternal = useCallback((hoverIndex: number) => {
    onHover && onHover(datapoints[hoverIndex]);
  }, [onHover, datapoints]);

  const config = useMemo(() => getShotChartConfig(theme, onHoverInternal), [theme, onHoverInternal]);
  const [chartData, setChartData] = useState(mapToChartData([], theme));

  if (newDataPoint && data) {
    throw new Error("Only one of 'newDataPoint' or 'data' props must be defined");
  }

  // Refreshes the chart completely by reseting the chartData. This redraws the whole chart.
  useEffect(() => {
    if (data === undefined || data === null) {
      return;
    }
    setDatapoints(data);
    setChartData(mapToChartData(data, theme));
  }, [data, theme, setDatapoints, setChartData]);

  // Adds newDataPoint to the end of the chart unless it detects that a new shot was started. More efficient.
  useEffect(() => {
    if (newDataPoint === undefined || newDataPoint === null) {
      return;
    }
    if (newShotStarted(newDataPoint, datapoints)) {
      setChartData(mapToChartData([newDataPoint], theme));
      setDatapoints([newDataPoint]);
    } else {
      addDataPointToChartData(chartData, newDataPoint, maxLength, chartRef);
      setDatapoints([...datapoints, newDataPoint]);
    }
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [newDataPoint, theme, maxLength, setDatapoints, setChartData]);

  return (
    <Line
      ref={chartRef}
      options={config}
      data={chartData}
    />
  );
}

export default ShotChart;
