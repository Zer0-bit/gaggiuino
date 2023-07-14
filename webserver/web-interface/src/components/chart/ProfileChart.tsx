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
  ChartDataset,
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
  labels: number[];
  pressureTargetsData: (number | null)[],
  pressureLimitsData: (number | null)[],
  flowTargetsData: (number | null)[],
  flowLimitsData: (number | null)[],
  tempData: (number | null)[],
  phaseIds: number[],
}

function profileToDatasets(profile: Profile): DataPoints {
  const data: DataPoints = {
    labels: [],
    pressureTargetsData: [],
    pressureLimitsData: [],
    flowTargetsData: [],
    flowLimitsData: [],
    tempData: [],
    phaseIds: [],
  };

  let phaseStartTime = 0;
  let currentSegment = 0;
  let phaseId = 0;
  profile.phases.forEach((phase) => {
    if (!phase.skip) {
      const lastDataIdx = data.labels.length - 1;
      const previousPressure = data.pressureTargetsData[lastDataIdx] || data.pressureLimitsData[lastDataIdx] || 0;
      const previousFlow = data.flowTargetsData[lastDataIdx] || data.flowLimitsData[lastDataIdx] || 0;
      const phaseTemp = (phase.waterTemperature) ? phase.waterTemperature : profile.waterTemperature;
      const previousTemp = currentSegment === 0 ? phaseTemp : data.tempData[lastDataIdx];

      const pressureTargets = getPressureTargets(phase, previousPressure);
      const pressureLimits = getPressureLimits(phase, previousPressure);
      const flowTargets = getFlowTargets(phase, previousFlow);
      const flowLimits = getFlowLimits(phase, previousFlow);
      const labels = getPointsInTime(phase, phaseStartTime);
      const temp = [previousTemp, ...Array(POINTS_PER_PHASE + 1).fill(phaseTemp)];
      const phaseIds = Array(POINTS_PER_PHASE + 1).fill(phaseId);

      data.labels.push(...labels);
      data.flowTargetsData.push(...flowTargets);
      data.flowLimitsData.push(...flowLimits);
      data.pressureTargetsData.push(...pressureTargets);
      data.pressureLimitsData.push(...pressureLimits);
      data.tempData.push(...temp);
      data.phaseIds.push(...phaseIds);

      phaseStartTime = labels[labels.length - 1];
      currentSegment += 1;
    }
    phaseId += 1;
  });

  return data;
}

function buildEmptyChartData(theme: Theme): ChartData<'line'> {
  return {
    labels: [],
    datasets: [
      {
        label: 'Pressure Target',
        data: [],
        backgroundColor: alpha(theme.palette.pressure.main, 0.8),
        borderColor: theme.palette.pressure.main,
        yAxisID: 'y2',
      },
      {
        label: 'Flow Target',
        data: [],
        backgroundColor: alpha(theme.palette.flow.main, 0.8),
        borderColor: theme.palette.flow.main,
        yAxisID: 'y2',
      },
      {
        label: 'Temp Target',
        data: [],
        backgroundColor: alpha(theme.palette.temperature.main, 0.8),
        borderColor: theme.palette.temperature.main,
        yAxisID: 'y1',
      },
      {
        label: 'Pressure Limit',
        data: [],
        backgroundColor: alpha(theme.palette.pressure.main, 0.8),
        borderColor: theme.palette.pressure.main,
        borderDash: [3, 3],
        yAxisID: 'y2',
      },
      {
        label: 'Flow Limit',
        data: [],
        backgroundColor: alpha(theme.palette.flow.main, 0.8),
        borderColor: theme.palette.flow.main,
        borderDash: [3, 3],
        yAxisID: 'y2',
      },
    ],
  };
}

function mapToChartData(data: DataPoints, theme: Theme, selectedPhaseIndex?: number): ChartData<'line'> {
  const nullSelected = (d: number | null, i: number) => (
    (selectedPhaseIndex !== undefined && selectedPhaseIndex === data.phaseIds[i]) ? null : d
  );
  const nullUnselected = (d: number | null, i: number) => (
    (selectedPhaseIndex !== undefined && selectedPhaseIndex === data.phaseIds[i]) ? d : null
  );
  const chartData = buildEmptyChartData(theme);
  chartData.labels = data.labels;
  chartData.datasets[0].data = data.pressureTargetsData.map(nullSelected);
  chartData.datasets[1].data = data.flowTargetsData.map(nullSelected);
  chartData.datasets[2].data = data.tempData.map(nullSelected);
  chartData.datasets[3].data = data.pressureLimitsData.map(nullSelected);
  chartData.datasets[4].data = data.flowLimitsData.map(nullSelected);

  if (selectedPhaseIndex !== undefined) {
    const selectedDataSets:ChartDataset<'line'>[] = chartData.datasets.map((dataSet) => ({
      ...dataSet, data: [], label: `Selected ${dataSet.label}`, borderWidth: 3,
    }));
    selectedDataSets[0].data = data.pressureTargetsData.map(nullUnselected);
    selectedDataSets[1].data = data.flowTargetsData.map(nullUnselected);
    selectedDataSets[2].data = data.tempData.map(nullUnselected);
    selectedDataSets[3].data = data.pressureLimitsData.map(nullUnselected);
    selectedDataSets[4].data = data.flowLimitsData.map(nullUnselected);
    chartData.datasets.push(...selectedDataSets);
  }

  return chartData;
}

export interface ProfileChartProps {
  profile: Profile;
  selectedPhaseIndex?: number;
  onSelectPhase?: (phaseIndex: number) => void;
}

function ProfileChart({ profile, selectedPhaseIndex = undefined, onSelectPhase = undefined }: ProfileChartProps) {
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

  const config = useMemo(
    () => getProfilePreviewChartConfig({
      theme,
      onClick: onClickHandler,
      max: datapoints.labels[datapoints.labels.length - 1],
    }),
    [theme, datapoints, onClickHandler],
  );

  return (
    <Line
      ref={chartRef}
      options={config}
      data={chartData}
    />
  );
}

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

function percentageWithCurve(pct: number, curve:CurveStyle) {
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
  curve: CurveStyle,
): number {
  const deltaRef = refEnd - refStart;
  const deltaTarget = targetEnd - targetStart;

  if (deltaRef === 0) {
    return targetEnd;
  }

  const pct = Math.max(0, Math.min(1, Math.abs((refNumber - refStart) / deltaRef)));

  return targetStart + deltaTarget * percentageWithCurve(pct, curve);
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
    pointsInTime.push(phaseStartTime + pct * Math.min(times.transition, times.totalTime));
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
  const times = getPhaseTimes(phase);
  const pointsInTime = getPointsInTime(phase, 0);

  const targetValues = [previousValue]; // Adding one extra point to connect to precious phase
  for (let i = 1; i < pointsInTime.length; i++) { // Skip first element as it's manually set to point to previousValue above
    targetValues.push(
      mapRange(pointsInTime[i], 0, times.transition, startingValue, endValue, phase.target.curve),
    );
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

function getPressureTargets(phase: Phase, previousPressure: number): (number | null)[] {
  if (phase.type === PhaseType.PRESSURE) {
    return getTargetValues(phase, previousPressure);
  }
  return [null, ...Array(POINTS_PER_PHASE).fill(null)];
}

function getPressureLimits(phase: Phase, previousPressure: number): (number | null)[] {
  if (phase.type === PhaseType.FLOW) {
    return [previousPressure, ...Array(POINTS_PER_PHASE).fill(phase.restriction || 0)];
  }
  return [null, ...Array(POINTS_PER_PHASE).fill(null)];
}

function getFlowTargets(phase: Phase, previousFlow: number): (number | null)[] {
  if (phase.type === PhaseType.FLOW) {
    return getTargetValues(phase, previousFlow);
  }
  return [null, ...Array(POINTS_PER_PHASE).fill(null)];
}

function getFlowLimits(phase: Phase, previousFlow: number): (number | null)[] {
  if (phase.type === PhaseType.PRESSURE) {
    return [previousFlow, ...Array(POINTS_PER_PHASE).fill(phase.restriction || 0)];
  }
  return [null, ...Array(POINTS_PER_PHASE).fill(null)];
}
