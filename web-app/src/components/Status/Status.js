import styled, { keyframes } from 'styled-components';
import StatusBase from './StatusBase';

const media = {
  desktop: '@media(min-width: 850px)',
  largedesktop: '@media(min-width: 1600px)'
}

const heartbeat = keyframes`
  0%
  {transform: font( .75 );}
  20%
  {transform: scale( 1 );}
  40%
  {transform: scale( .75 );}
  60%
  {transform: scale( 1 );}
  80%
  {transform: scale( .75 );}
  100%
  {transform: scale( .75 );}
`;

const Status = styled(StatusBase)`
  margin: 0 auto;
  font-family: 'Monserrat', sans-serif;
  text-align: left;
  padding: 5em 10em 3em 0;

  .table {
    display: table;
    margin: 0 auto;
  }

  ul {
    min-width: 696px;
	  list-style: none;
	  padding-top: 20px;
  }

  ul li {
		display: inline;
	}

  .status {
    font-size: 2em;
    padding-left: 2em;
    animation-name: ${heartbeat} 1s infinite;
  }
`;

export default Status;
