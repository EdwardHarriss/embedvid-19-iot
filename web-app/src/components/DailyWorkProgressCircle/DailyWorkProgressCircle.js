import styled from 'styled-components';
import DailyWorkProgressCircleBase from './DailyWorkProgressCircleBase';

const DailyWorkProgressCircle = styled(DailyWorkProgressCircleBase)`
  margin: 0 auto;
  padding: 4em;

  svg {
    height: 15em;
  }

  .chart-text {
    fill: #fafafa;
    transform: translateY(0.25em);
  }
  .chart-number {
    font-size: 0.55em;
    line-height: 1;
    text-anchor: middle;
    transform: translateY(-0.25em);
  }
  .chart-label {
    font-size: 0.2em;
    text-anchor: middle;
    transform: translateY(0.7em);
  }
  .review-text {
    font-size: 1em;
    color: #fafafa;
    text-anchor: middle;
  }
`;

export default DailyWorkProgressCircle;