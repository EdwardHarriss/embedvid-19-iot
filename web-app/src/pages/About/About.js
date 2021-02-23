import React, { Component } from 'react';
import ReactPlayer from 'react-player'
import Header from '../.././components/Header/Header';
import uses from '../../images/uses.png';
import graphic from '../../images/productivity.jpg';
import './About.css'

class About extends Component {
  render () {
    return (
      <div className="container">
        <Header page={'About'}/>
        <h2>About</h2>
        <p className="first-p">Consistent hard work can be tough, especially in unprecedented times like the current pandemic.</p>
        <p>Even with the best intentions, trying to maintain a productive work ethic doesn't always pan out as expected.</p>
        <p>Today's busy lifestyles add to the challenge and lead many thinking to themselves, <i>"How can I get the most out of my days?"</i></p>
        <p>Thanks to our product, all those grafters out there now have a helping hand.</p>
        <h4>From the embedvid-19 team, we are proud to present to you, <i><b>WorkMate!</b></i></h4>
        <ReactPlayer
          className="ad-vid"
          width="80%"
          url="https://www.youtube.com/watch?v=gsCVpNI1cgU&feature=youtu.be&ab_channel=WorkMate"
        />
        <p className="after-media-p">Now you can get even more satisfaction out of knowing you've hit your targets for the day, and a <i>little</i> push to help you if you haven't!</p>
        <p>Not only does WorkMate track your desk-hours, but it also monitors your surrounding environent and even helps you fix your posture - giving you the power to set up the perfect work environment.</p>
        <p><b>You can view all of this handy data on the website!</b></p>
        <p>WorkMate can be used in many different scenarios to help increase productivity, happiness and life!</p>
        <img src={graphic} alt="productivity graphic" id="productivity" className="productivity"></img>
        <img src={uses} alt="uses graphic" id="uses" className="uses"></img>
        <p className="after-media-p">Our product is perfect for individuals and corporations alike.</p>
        <p>Worried your workforce isn't achieving their full potential? Think someone isn't pulling their weight? <i>Well now you can prove it!</i></p>
        <p>With our Business Package, you can deploy WorkMates to all your employees, and supervise them from our easy-to-use web app.</p>
        <p>Remote management has never been so simple.</p>
        <h4><b>How To Use</b></h4>
        <ReactPlayer
          className="demo-vid"
          width="80%"
          url="https://www.youtube.com/watch?v=XtD7BT0j9Lg&feature=youtu.be&ab_channel=WorkMate"
        />
      </div>
    );
  }
}

export default About;