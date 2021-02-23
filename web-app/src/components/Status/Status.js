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
  font-family: 'Monserrat', sans-serif;
  text-align: center;
  
  .title {
    font-size: 1.4em;
    padding: 4em 10em 0 0;
    ${media.desktop}{
      padding-right: 0;
    }
  }

  .status {
    font-size: 2.5em;
    animation: ${heartbeat} 5s linear infinite;
    text-align: center;
  }
`;

export default Status;
