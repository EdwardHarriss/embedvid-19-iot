import React, { Component } from 'react';
import ReactPlayer from 'react-player'
import Header from '../.././components/Header/Header';
import graphic from '../../images/productivity.jpg'
import './About.css'

class About extends Component {
  render () {
    return (
      <div className="container">
        <Header page={'About'}/>
        <h5>From the embedvid-19 team, we are proud to present to you, WorkMate!</h5>
        <ReactPlayer 
          className="ad-vid"
          width="80%"
          url="https://www.youtube.com/watch?v=gsCVpNI1cgU&feature=youtu.be&ab_channel=WorkMate"
        />
        <p><span className="top-para">WorkMate is the innovative new device which tracks the time you spend at your desk working and rewards you for meeting your daily target.</span></p>
        <p>WorkMate can be used in many different scenarios to help increase productivity, happiness and life!</p>
        <img src={graphic} alt="productivity graphic" id="graphic" className="graphic"></img>
        <p>Not only does the device track time spent at your desk, but also your posture and room temperature, giving you the power to set up the optimal work environment.</p>
        <p>You can view all of this handy data on the website!</p>
        <h3>How To Use</h3>
        <ReactPlayer 
          className="demo-vid"
          width="80%"
          url="https://www.youtube.com/watch?v=gsCVpNI1cgU&feature=youtu.be&ab_channel=WorkMate"
        />
      </div>
    );
  }
}

export default About;
