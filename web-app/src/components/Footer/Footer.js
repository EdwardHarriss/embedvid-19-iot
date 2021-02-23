import styled, {css} from 'styled-components';
import FooterBase from './FooterBase';

const media = {
  desktop: '@media(min-width: 1000px)',
}

const Footer = styled(FooterBase)`
    margin: 0;
    padding: 0;
    font-family: 'Monserrat', sans-serif;

    .team {
        color: #98999e;
        display: block;
        padding-top: 1em;
        text-align: center;
        background-color: #2b3033;
        width: 100%;
        height: 3em;
        ${media.desktop}{
           height: 4em;
        }
`;
 


export default Footer;
