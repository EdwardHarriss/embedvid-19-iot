import styled, {css} from 'styled-components';
import HeaderBase from './HeaderBase';

const media = {
  desktop: '@media(min-width: 850px)',
  largedesktop: '@media(min-width: 1600px)'
}

window.onload=function(){
  var menu = document.getElementById('menu')
  var nav = document.getElementById('nav')
  var exit = document.getElementById('exit')

  if(menu){
    menu.addEventListener('click', function(e) {
      nav.classList.toggle('hide-mobile')
      e.preventDefault()
    });
  }

  if(exit){
    exit.addEventListener('click', function(e) {
      nav.classList.add('hide-mobile')
      e.preventDefault()
    });
  }
}

const Header = styled(HeaderBase)`
  margin: 0 auto;
  font-family: 'Monserrat', sans-serif;
  background-color: #2b3032;


  header {
    display: flex;
    justify-content: space-between;
  }

  header a {
    color: #fafafa;
  }

  .logo {
    margin: 1.2em;
  }

  ul {
    list-style-type: none;
    margin: 0;
    padding: .8em 1.2em;
    text-align: center;
  }

  .menu {
    width: 25px;
    margin: 1.5em 1em 0 0;
  } 

  .hide-mobile {
    display: none;

    ${media.desktop}{
      display: block;
    }
  }

  .hide-desktop{
    ${media.desktop}{
      display: none;
    }
  }

  nav ul {
    position: fixed;
    width: 50%;
    top: 0;
    right: 0;
    background: #141414;
    height: 16em;
    border-radius:0 0 0 15px;
    z-index: 7;
    padding-top: 3em;

    ${media.desktop}{
      position: inherit;
      width: auto;
      background: none;
      height: auto;
      display: flex;
      padding-top: 0;
    }
  } 

  nav ul li {
    ${media.desktop}{
      float: left;
      padding: .8em 2.3em;
    }
  }

  nav ul li a {
    color: #fafafa;
    display: block;
    text-decoration: none;
    width: 100%;
    padding: 1em 0em;
    border-radius:15px;

    ${media.desktop}{
      background-color: inherit;
      text-align: right;
    }
  }

  nav ul li a:hover {
    background-color: #2b3033;
    ${media.desktop}{
      background-color: inherit;
      color: #98999e;
    }
  }

  .exit-btn {
    margin-top: -1.3em;
    margin-bottom: 1em;
    text-align: right;
  }
  
  .exit-btn img {
    cursor: pointer;
  }
`;

export default Header;
