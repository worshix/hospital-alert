import { useState, useEffect, useCallback } from 'react'

interface Emergency {
  id: number
  createdAt: string
  updatedAt: string
  patientName: string
  location: string
}

export function useEmergencyDetection(streamUrl: string) {
  const [isEmergency, setIsEmergency] = useState(false)
  const [emergencies, setEmergencies] = useState<Emergency[]>([])
  const [lastEmergencyTime, setLastEmergencyTime] = useState<Date | null>(null)
  const [isLoading, setIsLoading] = useState(false)

  // Function to check if stream is available
  const checkStreamAvailability = useCallback(async () => {
    try {
      await fetch(streamUrl, { 
        method: 'HEAD',
        mode: 'no-cors',
        cache: 'no-cache'
      })
      return true
    } catch {
      // Try with a simple image request to test connectivity
      return new Promise<boolean>((resolve) => {
        const img = new Image()
        img.onload = () => resolve(true)
        img.onerror = () => resolve(false)
        img.src = streamUrl + '?' + Date.now() // Cache busting
        
        // Timeout after 3 seconds
        setTimeout(() => resolve(false), 3000)
      })
    }
  }, [streamUrl])

  // Function to log emergency to database
  const logEmergency = useCallback(async () => {
    try {
      const response = await fetch('/api/emergencies', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          patientName: 'Patient in Ward 1',
          location: 'Ward 1'
        })
      })
      
      if (response.ok) {
        const newEmergency = await response.json()
        setEmergencies(prev => [newEmergency, ...prev])
        setLastEmergencyTime(new Date())
      }
    } catch (error) {
      console.error('Failed to log emergency:', error)
    }
  }, [])

  // Function to fetch all emergencies
  const fetchEmergencies = useCallback(async () => {
    setIsLoading(true)
    try {
      const response = await fetch('/api/emergencies')
      if (response.ok) {
        const data = await response.json()
        setEmergencies(data)
      }
    } catch (error) {
      console.error('Failed to fetch emergencies:', error)
    } finally {
      setIsLoading(false)
    }
  }, [])

  // Monitor stream availability
  useEffect(() => {
    let wasEmergency = false

    const monitorStream = async () => {
      const streamAvailable = await checkStreamAvailability()
      
      // Emergency detected (stream started)
      if (streamAvailable && !wasEmergency) {
        setIsEmergency(true)
        await logEmergency()
        wasEmergency = true
      }
      // Emergency cleared (stream stopped)
      else if (!streamAvailable && wasEmergency) {
        setIsEmergency(false)
        wasEmergency = false
      }
    }

    // Check every 2 seconds
    const intervalId = setInterval(monitorStream, 2000)
    
    // Initial check
    monitorStream()

    return () => {
      if (intervalId) clearInterval(intervalId)
    }
  }, [checkStreamAvailability, logEmergency])

  // Fetch emergencies on mount
  useEffect(() => {
    fetchEmergencies()
  }, [fetchEmergencies])

  return {
    isEmergency,
    emergencies,
    lastEmergencyTime,
    isLoading,
    refetchEmergencies: fetchEmergencies
  }
}
