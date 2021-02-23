import React from 'react';
import { Link, withRouter } from 'react-router-dom'
import logo from '../../images/WorkMate.svg'
import menu from '../../images/menu-button.svg'
import exit from '../../images/exit-button.svg'

const HeaderBase = ({
  className,
  page
}) => {
  if(page == "Home") {
    return (
      <div className={className}>
        <div className="container">
          <header>
            <Link to="/">
              <img src={logo} alt="WorkMate" id="logo" className="logo"></img>
            </Link>

            <nav>
              <a href="" className="hide-desktop">
                <img src={menu} alt="toggle menu" id="menu" className="menu"></img>
              </a>
              <ul className="show-desktop-hide-mobile" id="nav">
                <li id="exit" className="exit-btn hide-desktop">
                  <img src={exit} alt="exit menu" ></img>
                </li>
                <li><Link to="/about">About</Link></li>
                <li><Link to="/#daily">Daily</Link></li>
                <li><Link to="/#weekly">Weekly</Link></li>
              </ul>
            </nav>

          </header>
        </div>
      </div>
    );
  }
  else if(page == "About"){
      return (
      <div className={className}>
        <div className="container">
          <header>
            <Link to="/">
              <img src={logo} alt="WorkMate" id="logo" className="logo"></img>
            </Link>

            <nav>
              <a href="" className="hide-desktop">
                <img src={menu} alt="toggle menu" id="menu" className="menu"></img>
              </a>
              <ul className="show-desktop-hide-mobile" id="nav">
                <li id="exit" className="exit-btn hide-desktop">
                  <img src={exit} alt="exit menu" ></img>
                </li>
                <li><Link to="/">Home</Link></li>
                <li><Link to="/">Daily</Link></li>
                <li><Link to="/">Weekly</Link></li>
              </ul>
            </nav>

          </header>
        </div>
      </div>
    );
  }
}

export default HeaderBase;

