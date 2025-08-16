import { NextRequest, NextResponse } from 'next/server'
import { PrismaClient } from '@/lib/generated/prisma'

const prisma = new PrismaClient()

// GET - Fetch all emergencies
export async function GET() {
  try {
    const emergencies = await prisma.emergency.findMany({
      orderBy: {
        createdAt: 'desc'
      }
    })
    
    return NextResponse.json(emergencies)
  } catch (error) {
    console.error('Error fetching emergencies:', error)
    return NextResponse.json(
      { error: 'Failed to fetch emergencies' },
      { status: 500 }
    )
  }
}

// POST - Create new emergency
export async function POST(request: NextRequest) {
  try {
    const body = await request.json()
    const { patientName, location } = body
    
    const emergency = await prisma.emergency.create({
      data: {
        patientName: patientName || 'Unknown Patient',
        location: location || 'Unknown Location'
      }
    })
    
    return NextResponse.json(emergency)
  } catch (error) {
    console.error('Error creating emergency:', error)
    return NextResponse.json(
      { error: 'Failed to create emergency' },
      { status: 500 }
    )
  }
}
