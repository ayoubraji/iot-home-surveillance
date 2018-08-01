__author__ = 'Ayoub Raji'
__credits__ = 'Roberto Vezzani'

import os, os.path
import random
import sqlite3
import string
import time
import datetime

import cherrypy

DB_STRING = "IoT.db"


class IoTGenerator(object):
    @cherrypy.expose
    def index(self):
        return open('index.html')


@cherrypy.expose
class SensorDataCollection(object):

    @cherrypy.tools.accept(media='text/html')
    def GET(self):
        with sqlite3.connect(DB_STRING) as c:
            cherrypy.session['ts'] = time.time()
            cursorDoor = c.execute("SELECT * FROM sensorsData WHERE Type='door' ORDER BY id DESC LIMIT 1")
            cursorAlarm = c.execute("SELECT * FROM sensorsData WHERE Type='alarm' ORDER BY id DESC LIMIT 1")

            resultDoor = cursorDoor.fetchone()
            resultAlarm = cursorAlarm.fetchone()

            #Create HTML code
            output="<ul>"
            if resultDoor != None:
                output += "<li> <b>Door</b> sensor value: <b>" + str(resultDoor[3]) + "</b> - Acquired on " + str(resultDoor[4]) + "</li>"
            else:
                output += "<li> Door sensor value not yet available </li>"
            if resultAlarm != None:
                output += "<li> <b>Alarm</b> sensor value: <b>" + str(resultAlarm[3]) + "</b> - Acquired on " + str(resultAlarm[4]) + "</li>"
            else:
                output += "<li> Alarm sensor value not yet available </li>"
            output+="</ul>"

            return output

    def POST(self, **params):
        value = params['value']
        type = params['type']
        sensorid = params['sensorid']
        ts = time.time()
        st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
        with sqlite3.connect(DB_STRING) as c:
            cherrypy.session['ts'] = time.time()
            c.execute("INSERT INTO sensorsData (Type, SensorId, Value, AcquiredOn) VALUES ('"+ type + "','" + sensorid + "','" + value + "','" + st + "')")
        return "ok"


def setup_database():
    """
    Create the `sensorsData` table in the database
    on server startup
    """
    try:
        with sqlite3.connect(DB_STRING) as con:
            cleanup_database()
            con.execute("CREATE TABLE sensorsData (Id INTEGER PRIMARY KEY AUTOINCREMENT, Type, SensorId, Value, AcquiredOn)")
    except:
        pass


def cleanup_database():
    """
        Cleanup the `sensorsData` table in the database
        on server shutdown
        """
    try:
        with sqlite3.connect(DB_STRING) as con:
            con.execute(
                "DELETE FROM sensorsData")
    except:
        pass

if __name__ == '__main__':
    conf = {
        'global':{
            'server.socket_host': 'localhost',
            'server.socket_port': 80,
        },
        '/': {
            'tools.sessions.on': True,
            'tools.staticdir.root': os.path.abspath(os.getcwd()),
        },
        '/surveillanceStatus': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tools.response_headers.on': True,
            'tools.response_headers.headers': [('Content-Type', 'text/html')],
        },
        '/static': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': './static'
        }

    }

    cherrypy.engine.subscribe('start', setup_database)
    cherrypy.engine.subscribe('stop', cleanup_database)

    webapp = IoTGenerator()
    webapp.surveillanceStatus = SensorDataCollection()

    cherrypy.quickstart(webapp, '/', conf)