import styled, { keyframes } from 'styled-components';
import StatusBase from './StatusBase';

const media = {
  desktop: '@media(min-width: 850px)',
  largedesktop: '@media(min-width: 1600px)'
}

const heartbeat = keyframes`
  0%
  {transform: scale(.9);}
  20%
  {transform: scale( 1 );}
  40%
  {transform: scale( .9 );}
  60%
  {transform: scale( 1 );}
  80%
  {transform: scale( .9 );}
  100%
  {transform: scale( .9 );}
`;

const Status = styled(StatusBase)`
  margin: 0 auto;
  padding: 3em 0 5em 0;
  font-family: 'Monserrat', sans-serif;
  text-align: center;

  ${media.largedesktop}{
    padding: 4em 0;
  }
  
  .title {
    font-size: 1.4em;
    padding-top: 5em;
  }

  .status {
    font-size: 3.7em;
    padding-top: .2em;
    animation: ${heartbeat} 5s linear infinite;
    text-align: center;
  }
`;

export default Status;
