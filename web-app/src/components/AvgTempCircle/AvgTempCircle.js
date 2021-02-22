import styled from 'styled-components';
import AvgTempCircleBase from './AvgTempCircleBase';

const media = {
  phone: '@media(min-width: 450px)',
  tablet: '@media(min-width: 600px)',
  desktop: '@media(min-width: 850px)',
  largedesktop: '@media(min-width: 1600px)'
}

const AvgTempCircle = styled(AvgTempCircleBase)`
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
    font-size: 0.6em;
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
    padding: 0 1.5em;

    ${media.phone}{
      padding: 0 1.5em;
    }
    ${media.tablet}{
      padding: 0 1.5em;
    }
  }
`;

export default AvgTempCircle;