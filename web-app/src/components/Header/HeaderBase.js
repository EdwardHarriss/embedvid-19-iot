import React from 'react';
import logo from '../../images/WorkMate.svg'
import menu from '../../images/menu-button.svg'
import exit from '../../images/exit-button.svg'

const HeaderBase = ({
  className
}) => {
  return (
    <div className={className}>
      <div className="container">
        <header>
          <a href="">
            <img src={logo} alt="WorkMate" id="logo" className="logo"></img>
          </a>

          <nav>
            <a href="#" className="hide-desktop">
              <img src={menu} alt="toggle menu" id="menu" className="menu"></img>
            </a>
            <ul className="show-desktop-hide-mobile" id="nav">
              <li id="exit" className="exit-btn hide-desktop">
                <img src={exit} alt="exit menu" ></img>
              </li>
              <li><a href="#">Setup</a></li>
              <li><a href="#">Daily</a></li>
              <li><a href="#">Weekly</a></li>
            </ul>
          </nav>

        </header>
      </div>
    </div>
  );
}

export default HeaderBase;

