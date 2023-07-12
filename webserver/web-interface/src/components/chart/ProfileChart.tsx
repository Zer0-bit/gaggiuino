import React, {
  useRef, useMemo, useCallback,
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
  PhaseType, Phase, Profile, CurveStyle,
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

const POINTS_PER_PHASE = 10;

interface DataPoints {
  labels: Array<number>;
  pressureData: Array<number>,
  flowData: Array<number>,
  tempData: Array<number>,
  phaseIds: Array<number>,
}

function profileToDatasets(profile: Profile): DataPoints {
  const data: DataPoints = {
    labels: [],
    pressureData: [],
    flowData: [],
    tempData: [],
    phaseIds: [],
  };

  let phaseStartTime = 0;
  let currentSegment = 0;
  let phaseId = 0;
  profile.phases.forEach((phase) => {
    if (!phase.skip) {
      const previousPressure = currentSegment === 0 ? 0 : data.pressureData[data.pressureData.length - 1];
      const previousFlow = currentSegment === 0 ? 0 : data.flowData[data.pressureData.length - 1];
      const phaseTemp = (phase.waterTemperature) ? phase.waterTemperature : profile.waterTemperature;
      const previousTemp = currentSegment === 0 ? phaseTemp : data.tempData[data.tempData.length - 1];

      const pressureTargets = getPressureSegment(phase, previousPressure);
      const flowTargets = getFlowSegment(phase, previousFlow);
      const labels = getPointsInTime(phase, phaseStartTime);
      const temp = [previousTemp, ...Array(POINTS_PER_PHASE + 1).fill(phaseTemp)];
      const phaseIds = Array(POINTS_PER_PHASE + 1).fill(phaseId);

      data.labels.push(...labels);
      data.flowData.push(...flowTargets);
      data.pressureData.push(...pressureTargets);
      data.tempData.push(...temp);
      data.phaseIds.push(...phaseIds);

      phaseStartTime = labels[labels.length - 1];
      currentSegment += 1;
    }
    phaseId += 1;
  });

  return data;
}

function mapToChartData(data: DataPoints, theme: Theme, selectedPhaseIndex?: number): ChartData<'line'> {
  const nullSelected = (d: number, i: number) => (
    (selectedPhaseIndex !== undefined && selectedPhaseIndex === data.phaseIds[i]) ? null : d
  );
  const nullUnselected = (d: number, i: number) => (
    (selectedPhaseIndex !== undefined && selectedPhaseIndex === data.phaseIds[i]) ? d : null
  );
  return {
    labels: data.labels,
    datasets: [
      {
        label: 'Pressure',
        data: data.pressureData.map(nullSelected),
        backgroundColor: alpha(theme.palette.pressure.main, 0.8),
        borderColor: theme.palette.pressure.main,
        yAxisID: 'y2',
      },
      {
        label: 'Flow',
        data: data.flowData.map(nullSelected),
        backgroundColor: alpha(theme.palette.flow.main, 0.8),
        borderColor: theme.palette.flow.main,
        yAxisID: 'y2',
      },
      {
        label: 'Temp',
        data: data.tempData.map(nullSelected),
        backgroundColor: alpha(theme.palette.temperature.main, 0.8),
        borderColor: theme.palette.temperature.main,
        yAxisID: 'y1',
      },
      {
        label: 'Selected Pressure',
        data: data.pressureData.map(nullUnselected),
        backgroundColor: alpha(theme.palette.pressure.main, 0.8),
        borderColor: theme.palette.pressure.main,
        borderDash: [],
        borderWidth: 3,
        yAxisID: 'y2',
      },
      {
        label: 'Selected Flow',
        data: data.flowData.map(nullUnselected),
        backgroundColor: alpha(theme.palette.flow.main, 0.8),
        borderColor: theme.palette.flow.main,
        borderDash: [],
        borderWidth: 3,
        yAxisID: 'y2',
      },
      {
        label: 'Selected Temp',
        data: data.tempData.map(nullUnselected),
        backgroundColor: alpha(theme.palette.temperature.main, 0.8),
        borderColor: theme.palette.temperature.main,
        borderDash: [],
        borderWidth: 3,
        yAxisID: 'y1',
      },
    ],
  };
}

export interface ProfileChartProps {
  profile: Profile;
  selectedPhaseIndex?: number;
  onSelectPhase?: (phaseIndex: number) => void;
}

function ProfileChart({ profile, selectedPhaseIndex, onSelectPhase }: ProfileChartProps) {
  const chartRef = useRef(null);
  const theme = useTheme();
  const datapoints = useMemo(() => profileToDatasets(profile), [profile]);
  const chartData = useMemo(
    () => mapToChartData(datapoints, theme, selectedPhaseIndex),
    [datapoints, selectedPhaseIndex, theme],
  );
  const onClickHandler = useCallback((dataIndex: number) => {
    onSelectPhase && onSelectPhase(datapoints.phaseIds[dataIndex]);
  }, [datapoints, onSelectPhase]);
  const config = useMemo(() => getProfilePreviewChartConfig(theme, onClickHandler), [theme, onClickHandler]);

  return (
    <Line
      ref={chartRef}
      options={config}
      data={chartData}
    />
  );
}
ProfileChart.defaultProps = {
  selectedPhaseIndex: undefined,
  onSelectPhase: undefined,
};

export default ProfileChart;

function easeIn(pct: number) {
  return pct ** 1.675;
}

function easeOut(pct: number) {
  return 1 - (1 - pct) ** 1.675;
}

function easeInOut(pct: number) {
  return 0.5 * (Math.sin((pct - 0.5) * Math.PI) + 1);
}

function percentageWithTransition(pct: number, curve:CurveStyle) {
  switch (curve) {
    case CurveStyle.LINEAR:
      return pct;
    case CurveStyle.EASE_IN:
      return easeIn(pct);
    case CurveStyle.EASE_OUT:
      return easeOut(pct);
    case CurveStyle.EASE_IN_OUT:
      return easeInOut(pct);
    default:
      return 1;
  }
}

function mapRange(
  refNumber: number,
  refStart: number,
  refEnd: number,
  targetStart: number,
  targetEnd: number,
  transition: CurveStyle,
): number {
  const deltaRef = refEnd - refStart;
  const deltaTarget = targetEnd - targetStart;

  if (deltaRef === 0) {
    return targetEnd;
  }

  const pct = Math.max(0, Math.min(1, Math.abs((refNumber - refStart) / deltaRef)));

  return targetStart + deltaTarget * percentageWithTransition(pct, transition);
}

// Essentially this calculates the x axis(time) for a phase
// Breaks up the transition into `POINTS_PER_PHASE` points in time and returns these
// points in time.
function getPointsInTime(phase: Phase, phaseStartTime:number) {
  const times = getPhaseTimes(phase);
  const pointCounts = getPointCounts(phase);

  const pointsInTime = [phaseStartTime]; // Adding one extra point to connect to precious phase
  for (let i = 0; i < pointCounts.forTransition; i++) {
    const pct = i / (pointCounts.forTransition - 1);
    pointsInTime.push(phaseStartTime + pct * times.transition);
  }
  for (let i = 0; i < pointCounts.forRestOfPhase; i++) {
    const pct = i / (pointCounts.forRestOfPhase - 1);
    pointsInTime.push(phaseStartTime + times.transition + pct * times.restOfPhase);
  }
  return pointsInTime;
}

// Essentially this calculates the y axis for a metric (pressure / flow)
// It breaks up the transition into  `POINTS_PER_PHASE` points in time and returns the
// value the target is expected to have for these points in time.
function getTargetValues(phase: Phase, previousValue: number): number[] {
  const startingValue = (phase.target.start) ? phase.target.start : previousValue;
  const endValue = phase.target.end || 0;
  const pointCounts = getPointCounts(phase);

  const targetValues = [previousValue]; // Adding one extra point to connect to precious phase
  for (let i = 0; i < pointCounts.forTransition; i++) {
    const pct = i / (pointCounts.forTransition - 1);
    targetValues.push(mapRange(pct, 0, 1, startingValue, endValue, phase.target.curve));
  }
  for (let i = 0; i < pointCounts.forRestOfPhase; i++) {
    targetValues.push(endValue);
  }
  return targetValues;
}

function getPhaseTimes(phase: Phase) {
  const transition = phase.target.time || 0;
  const totalTime = phase.stopConditions.time || transition + 5000;

  return {
    transition,
    totalTime,
    restOfPhase: totalTime - transition,
  };
}

// Tries to split the phase into two equal froups of points for the transition and the rest (if necessary)
function getPointCounts(phase: Phase) {
  const times = getPhaseTimes(phase);

  let pointsForTransition = (times.restOfPhase > 0) ? POINTS_PER_PHASE / 2 : POINTS_PER_PHASE;
  if (times.transition === 0) {
    pointsForTransition = 0;
  }
  const pointsForRestOfPhase = POINTS_PER_PHASE - pointsForTransition;
  return {
    forTransition: pointsForTransition,
    forRestOfPhase: pointsForRestOfPhase,
  };
}

function getPressureSegment(phase: Phase, previousPressure: number): number[] {
  if (phase.type === PhaseType.FLOW) {
    return [previousPressure, ...Array(POINTS_PER_PHASE).fill(phase.restriction || 0)];
  }

  return getTargetValues(phase, previousPressure);
}

function getFlowSegment(phase: Phase, previousFlow: number): number[] {
  if (phase.type === PhaseType.PRESSURE) {
    return [previousFlow, ...Array(POINTS_PER_PHASE).fill(phase.restriction || 0)];
  }
  return getTargetValues(phase, previousFlow);
}
