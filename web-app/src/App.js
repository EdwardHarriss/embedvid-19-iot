import React, { Component } from 'react';
import { BrowserRouter as Router, Route, Switch } from 'react-router-dom'
import Header from './components/Header/Header';
import Home from './pages/Home/Home';
import About from './pages/About/About';
import Footer from './components/Footer/Footer';
import './App.css'

class App extends Component {
  render () {
    return (
      <div>
        <div className="container">
          <Router>
            <Switch>
              <Route path="/" exact component={() => <Home/>}/>
              <Route path="/about" exact component={() => <About/>}/>
            </Switch>
          </Router>
        </div>
        <div>
          <Footer/>
        </div>
      </div>
    );
  }
}

export default App;
