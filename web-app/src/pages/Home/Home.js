import React, { Component } from 'react';
import firebase from '../.././firebase.js';
import Header from '../.././components/Header/Header';
import Status from '../.././components/Status/Status';
import {DropdownButton, Dropdown} from 'react-bootstrap';
import DailyWorkProgressCircle from '../.././components/DailyWorkProgressCircle/DailyWorkProgressCircle';
import AvgTempCircle from '../.././components/AvgTempCircle/AvgTempCircle';
import DailyPostureLineChart from '../.././components/DailyPostureLineChart';
import WeeklyWorkBarChart from '../.././components/WeeklyWorkBarChart';
import WeeklyTempBarChart from '../.././components/WeeklyTempBarChart';
import './Home.css';

var now = new Date()


Date.prototype.getWeekNumber = function(){
  var d = new Date(Date.UTC(this.getFullYear(), this.getMonth(), this.getDate()));
  var dayNum = d.getUTCDay() || 7;
  d.setUTCDate(d.getUTCDate() + 4 - dayNum);
  var yearStart = new Date(Date.UTC(d.getUTCFullYear(),0,1));
  return Math.ceil((((d - yearStart) / 86400000) + 1)/7)
};


class Home extends Component {
  constructor(props) {
    super(props)

    this.state = {
      TargetHours: localStorage.getItem('TargetHours') || 0,
      items: [],
      weeklyItems: []
    }
  }

  componentDidMount() {
    const itemsRef = firebase.database().ref();
    itemsRef.on('value', (snapshot) => {
      let items = snapshot.val();
      let newState = [];
      let newWeeklyState = [];
      for (let item in items) {
        if (items[item] != "Hello") {
          let DataDate = new Date(JSON.parse(items[item]).time * 1000);
          if (DataDate.getDate() == now.getDate() && DataDate.getMonth() == now.getMonth() && DataDate.getFullYear() == now.getFullYear()) {
            newState.push({
              id: item,
              time: JSON.parse(items[item]).time,
              temp: JSON.parse(items[item]).temperature,
              distance: JSON.parse(items[item]).average_distance,
              awayFromDesk: JSON.parse(items[item]).away_from_desk
            });
          }
          if (DataDate.getWeekNumber() == now.getWeekNumber() && DataDate.getFullYear() == now.getFullYear()) {
            newWeeklyState.push({
              id: item,
              time: JSON.parse(items[item]).time,
              temp: JSON.parse(items[item]).temperature,
              distance: JSON.parse(items[item]).average_distance,
              awayFromDesk: JSON.parse(items[item]).away_from_desk
            });
          }
          this.setState({
            items: newState,
            weeklyItems: newWeeklyState
          });
        }
      }
    });
  }

  SetTargetHours = (hrs) => {
    localStorage.setItem('TargetHours', hrs);
    this.setState({TargetHours: hrs})
  }

  render () {
    return (
      <div className="container">
        <Header page={'Home'}/>
        <Status
          class
          timeData={this.state.items.map((item) => {return (item.time)})}
          awayDesk={this.state.items.map((item) => {return (item.awayFromDesk)})}
        />
        <DropdownButton id="target-hours-dropdown" title="Set Daily Hours of Work Target">
          <div id="dropdown-options">
          <Dropdown.Item onClick={() => this.SetTargetHours(0)} href="#/target-0">0</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(0.5)} href="#/target-0.5">0.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(1)} href="#/target-1">1</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(1.5)} href="#/target-1.5">1.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(2)} href="#/target-2">2</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(2.5)} href="#/target-2.5">2.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(3)} href="#/target-3">3</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(3.5)} href="#/target-3.5">3.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(4)} href="#/target-4">4</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(4.5)} href="#/target-4.5">4.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(5)} href="#/target-5">5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(5.5)} href="#/target-5.5">5.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(6)} href="#/target-6">6</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(6.5)} href="#/target-6.5">6.5</Dropdown.Item>            
          <Dropdown.Item onClick={() => this.SetTargetHours(7)} href="#/target-7">7</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(7.5)} href="#/target-7.5">7.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(8)} href="#/target-8">8</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(8.5)} href="#/target-8.5">8.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(9)} href="#/target-9">9</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(9.5)} href="#/target-9.5">9.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(10)} href="#/target-10">10</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(10.5)} href="#/target-10.5">10.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(11)} href="#/target-11">11</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(11.5)} href="#/target-11.5">11.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(12)} href="#/target-12">12</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(12.5)} href="#/target-12.5">12.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(13)} href="#/target-13">13</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(13.5)} href="#/target-13.5">13.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(14)} href="#/target-14">14</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(14.5)} href="#/target-14.5">14.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(15)} href="#/target-15">15</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(15.5)} href="#/target-15.5">15.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(16)} href="#/target-16">16</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(16.5)} href="#/target-16.5">16.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(17)} href="#/target-17">17</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(17.5)} href="#/target-17.5">17.5</Dropdown.Item>
          <Dropdown.Item onClick={() => this.SetTargetHours(18)} href="#/target-18">18</Dropdown.Item>
          </div>
        </DropdownButton>
        <DailyWorkProgressCircle
          className="dailyWorkTime"
          textColor="#fafafa"
          loadRingColour="#51cda0"
          loadRingWidth={2}
          innerText="Hours Worked."
          timeData={this.state.items.map((item) => {return (item.time)})}
          targetHours={this.state.TargetHours}
          fullRingWidth={3}
          fullRingColour="#6d78ad"
          awayDesk={this.state.items.map((item) => {return (item.awayFromDesk)})}
        />
        <AvgTempCircle
          className="dailyAvgTemp"
          textColor="#fafafa"
          innerText="Average Temp."
          tempData={this.state.items.map((item) => {return (item.temp)})}
          fullRingWidth={3}
          fullRingColour="#51cda0"
          awayDesk={this.state.items.map((item) => {return (item.awayFromDesk)})}
        />
        <div className="daily-posture-chart">
          <h3>Your Posture Today</h3>
          <DailyPostureLineChart
            distanceVals={this.state.items.map((item) => {return (item.distance)})}
            awayDesk={this.state.items.map((item) => {return (item.awayFromDesk)})}
          />
        </div>
        <div className="weekly-work-chart">
          <h3>Hours Worked This Week</h3>
          <WeeklyWorkBarChart
            targetHours={this.state.TargetHours}
            timeData={this.state.weeklyItems.map((item) => {return (item.time)})}
            awayDesk={this.state.weeklyItems.map((item) => {return (item.awayFromDesk)})}
          />
        </div>
        <div className="weekly-temp-chart">
          <h3>Average Temperatures This Week</h3>
          <WeeklyTempBarChart
            tempData={this.state.weeklyItems.map((item) => {return (item.temp)})}
            timeData={this.state.weeklyItems.map((item) => {return (item.time)})}
            awayDesk={this.state.weeklyItems.map((item) => {return (item.awayFromDesk)})}
          />
        </div>
      </div>
    );
  }
}

export default Home;
