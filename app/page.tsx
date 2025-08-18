"use client"

import { useState, useEffect } from "react"
import { AlertTriangle, Shield, Camera, Wifi, WifiOff } from "lucide-react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Alert, AlertDescription } from "@/components/ui/alert"
import { EmergencyTable } from "@/components/EmergencyTable"
import { useEmergencyDetection } from "@/hooks/useEmergencyDetection"

export default function HospitalAlertSystem() {
  const [cameraConnected, setCameraConnected] = useState(true)
  const [currentTime, setCurrentTime] = useState(new Date())

  // ESPCAM stream URL - replace with your actual ESPCAM IP address
  const ESPCAM_STREAM_URL = "http://192.168.43.163/stream"

  // Use the emergency detection hook
  const { isEmergency, emergencies, isLoading } = useEmergencyDetection(ESPCAM_STREAM_URL)

  useEffect(() => {
    const timer = setInterval(() => {
      setCurrentTime(new Date())
    }, 1000)

    return () => clearInterval(timer)
  }, [])

  const handleCameraError = () => {
    setCameraConnected(false)
  }

  const handleCameraLoad = () => {
    setCameraConnected(true)
  }

  return (
    <div
      className={`min-h-screen transition-colors duration-500 ${
        isEmergency ? "bg-red-50 dark:bg-red-950" : "bg-green-50 dark:bg-green-950"
      }`}
    >
      <div className="container mx-auto p-6 max-w-4xl">
        {/* Header */}
        <div className="flex items-center justify-between mb-6">
          <div className="flex items-center gap-3">
            <div
              className={`p-2 rounded-full ${
                isEmergency ? "bg-red-100 dark:bg-red-900" : "bg-green-100 dark:bg-green-900"
              }`}
            >
              {isEmergency ? (
                <AlertTriangle className="h-6 w-6 text-red-600 dark:text-red-400" />
              ) : (
                <Shield className="h-6 w-6 text-green-600 dark:text-green-400" />
              )}
            </div>
            <div>
              <h1 className="text-2xl font-bold text-gray-900 dark:text-gray-100">Hospital Alert System</h1>
              <p className="text-sm text-gray-600 dark:text-gray-400">{currentTime.toLocaleString()}</p>
            </div>
          </div>

          {/* Status Badge */}
          <Badge variant={isEmergency ? "destructive" : "default"} className="text-sm px-3 py-1">
            {isEmergency ? "🚨 EMERGENCY ACTIVE" : "✅ ALL SYSTEMS NORMAL"}
          </Badge>
        </div>

        {/* Main Content */}
        {isEmergency ? (
          <div className="space-y-6">
            {/* Emergency Alert */}
            <Alert className="border-red-200 bg-red-50 dark:border-red-800 dark:bg-red-950">
              <AlertTriangle className="h-5 w-5 text-red-600 dark:text-red-400" />
              <AlertDescription className="text-red-800 dark:text-red-200 font-semibold text-lg">
                🚨 EMERGENCY ALERT - Patient requires immediate attention
              </AlertDescription>
            </Alert>

            {/* Camera Feed */}
            <Card className="border-red-200 dark:border-red-800">
              <CardHeader className="pb-3">
                <div className="flex items-center justify-between">
                  <CardTitle className="flex items-center gap-2 text-red-800 dark:text-red-200">
                    <Camera className="h-5 w-5" />
                    Live Camera Feed - Patient Room
                  </CardTitle>
                  <Badge variant={cameraConnected ? "default" : "destructive"} className="flex items-center gap-1">
                    {cameraConnected ? <Wifi className="h-3 w-3" /> : <WifiOff className="h-3 w-3" />}
                    {cameraConnected ? "Connected" : "Disconnected"}
                  </Badge>
                </div>
              </CardHeader>
              <CardContent>
                <div className="relative bg-black rounded-lg overflow-hidden aspect-video">
                  {cameraConnected ? (
                    // eslint-disable-next-line @next/next/no-img-element
                    <img
                      src={ESPCAM_STREAM_URL}
                      alt="ESPCAM Live Feed"
                      className="w-full h-full object-cover block video-flip"
                      onError={handleCameraError}
                      onLoad={handleCameraLoad}
                    />
                  ) : (
                    <div className="flex items-center justify-center h-full text-white">
                      <div className="text-center">
                        <WifiOff className="h-12 w-12 mx-auto mb-2 text-red-400" />
                        <p className="text-lg font-semibold">Camera Feed Unavailable</p>
                        <p className="text-sm text-gray-300">Check ESPCAM connection</p>
                        <p className="text-xs text-gray-400 mt-2">URL: {ESPCAM_STREAM_URL}</p>
                      </div>
                    </div>
                  )}

                  {/* Emergency Overlay */}
                  <div className="absolute top-4 left-4 bg-red-600 text-white px-3 py-1 rounded-full text-sm font-bold animate-pulse">
                    🔴 LIVE - EMERGENCY
                  </div>
                </div>
              </CardContent>
            </Card>

            {/* Emergency Instructions */}
            <Card className="border-red-200 dark:border-red-800">
              <CardContent className="pt-6">
                <h3 className="font-semibold text-red-800 dark:text-red-200 mb-2">Emergency Protocol:</h3>
                <ul className="text-sm text-red-700 dark:text-red-300 space-y-1">
                  <li>• Medical staff has been automatically notified</li>
                  <li>• Emergency response team is en route</li>
                  <li>• Monitor patient condition via live feed</li>
                  <li>• Prepare necessary medical equipment</li>
                </ul>
              </CardContent>
            </Card>
          </div>
        ) : (
          /* Normal State */
          <div className="text-center py-12">
            <Card className="border-green-200 dark:border-green-800 max-w-md mx-auto">
              <CardContent className="pt-8 pb-8">
                <div className="mb-6">
                  <div className="mx-auto w-20 h-20 bg-green-100 dark:bg-green-900 rounded-full flex items-center justify-center mb-4">
                    <Shield className="h-10 w-10 text-green-600 dark:text-green-400" />
                  </div>
                  <h2 className="text-2xl font-bold text-green-800 dark:text-green-200 mb-2">All Systems Normal</h2>
                  <p className="text-green-700 dark:text-green-300">
                    Patient monitoring is active. No emergencies detected.
                  </p>
                </div>

                <div className="space-y-2 text-sm text-green-600 dark:text-green-400">
                  <div className="flex items-center justify-center gap-2">
                    <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse"></div>
                    <span>ESPCAM System Online</span>
                  </div>
                  <div className="flex items-center justify-center gap-2">
                    <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse"></div>
                    <span>Alert System Active</span>
                  </div>
                  <div className="flex items-center justify-center gap-2">
                    <div className="w-2 h-2 bg-green-500 rounded-full animate-pulse"></div>
                    <span>Monitoring Patient Room</span>
                  </div>
                </div>
              </CardContent>
            </Card>
          </div>
        )}

        {/* Emergency Logs Table */}
        <div className="mt-8">
          <EmergencyTable emergencies={emergencies} isLoading={isLoading} />
        </div>
      </div>
    </div>
  )
}
