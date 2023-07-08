import React, {
  useRef, useMemo, useState, useEffect,
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
import { getProfilePreviewChartConfig } from './ChartConfig';
import {
  PhaseType, Phase, Profile, Transition, CurveStyle,
} from '../../models/profile';

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

interface LineSegment {
  start:number;
  end: number;
}

function getTransitionValues(transition: Transition, previousValue: number): LineSegment {
  let startingValue = transition.start;
  if (transition.curve === CurveStyle.INSTANT || transition.time === 0 || transition.time === undefined) {
    startingValue = transition.end;
  }
  if (startingValue === 0 || startingValue === undefined) {
    startingValue = previousValue;
  }
  return { start: startingValue, end: transition.end };
}

function getPressureSegment(phase: Phase, previousPressure: number): LineSegment {
  if (phase.type === PhaseType.FLOW) {
    return { start: phase.restriction || 0, end: phase.restriction || 0 };
  }

  return getTransitionValues(phase.target, previousPressure);
}

function getFlowSegment(phase: Phase, previousFlow: number): LineSegment {
  if (phase.type === PhaseType.PRESSURE) {
    return { start: phase.restriction || 0, end: phase.restriction || 0 };
  }
  return getTransitionValues(phase.target, previousFlow);
}

interface DataPoints {
  labels: Array<number>;
  pressureData: Array<number>,
  flowData: Array<number>,
  tempData: Array<number>
}

function profileToDatasets(profile: Profile): DataPoints {
  const data: DataPoints = {
    labels: [],
    pressureData: [],
    flowData: [],
    tempData: [],
  };

  let phaseStartTime = 0;
  let currentSegment = 0;
  profile.phases.forEach((phase) => {
    if (!phase.skip) {
      const previousPressure = currentSegment === 0 ? 0 : data.pressureData[data.pressureData.length - 1];
      const previousFlow = currentSegment === 0 ? 0 : data.flowData[data.pressureData.length - 1];

      const transitionTime = phase.target.time || 0;
      const phaseTime = phase.stopConditions?.time || transitionTime + 5000;
      const pressureTargets = getPressureSegment(phase, previousPressure);
      const flowTargets = getFlowSegment(phase, previousFlow);
      let phaseTemp = (phase.waterTemperature || 0);
      phaseTemp = phaseTemp > 0 ? phaseTemp : profile.waterTemperature;

      data.labels.push(phaseStartTime / 1000);
      data.flowData.push(flowTargets.start);
      data.pressureData.push(pressureTargets.start);
      data.tempData.push(phaseTemp);

      if (transitionTime < phaseTime) {
        data.labels.push((phaseStartTime + transitionTime) / 1000);
        data.flowData.push(flowTargets.end);
        data.pressureData.push(pressureTargets.end);
        data.tempData.push(phaseTemp);
      }
      data.labels.push((phaseStartTime + phaseTime) / 1000);
      data.flowData.push(flowTargets.end);
      data.pressureData.push(pressureTargets.end);
      data.tempData.push(phaseTemp);

      phaseStartTime += phaseTime + 500;
      currentSegment += 1;
    }
  });

  return data;
}

function mapToChartData(profile: Profile, theme: Theme): ChartData<'line'> {
  const data = profileToDatasets(profile);
  return {
    labels: data.labels,
    datasets: [
      {
        label: 'Pressure',
        data: data.pressureData,
        backgroundColor: alpha(theme.palette.pressure.main, 0.8),
        borderColor: theme.palette.pressure.main,
        tension: 0.11,
        yAxisID: 'y2',
      },
      {
        label: 'Flow',
        data: data.flowData,
        backgroundColor: alpha(theme.palette.flow.main, 0.8),
        borderColor: theme.palette.flow.main,
        tension: 0,
        yAxisID: 'y2',
      },
      {
        label: 'Temp',
        data: data.tempData,
        backgroundColor: alpha(theme.palette.temperature.main, 0.8),
        borderColor: theme.palette.temperature.main,
        tension: 0.11,
        yAxisID: 'y1',
      },
    ],
  };
}

export interface ProfileChartProps {
  profile: Profile;
}

function ProfileChart({ profile }: ProfileChartProps) {
  const chartRef = useRef(null);
  const theme = useTheme();
  const config = useMemo(() => getProfilePreviewChartConfig(theme), [theme]);
  const [chartData, setChartData] = useState(mapToChartData(profile, theme));

  useEffect(() => {
    setChartData(mapToChartData(profile, theme));
  }, [profile, theme]);

  return (
    <Line
      ref={chartRef}
      options={config}
      data={chartData}
    />
  );
}

export default ProfileChart;
