import React, { Component } from 'react';
import Header from '../.././components/Header/Header';
import './About.css'

class About extends Component {
  render () {
    return (
      <div className="container">
        <Header page={'About'}/>
      </div>
    );
  }
}

export default About;
