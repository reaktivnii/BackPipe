pipeline {
    agent any

    environment {
        DOCKER_IMAGE = 'reaktivnii/backpipe:arm64'
        DOCKER_CREDENTIALS = credentials('docker-hub-creds')
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Build ARM64 Image') {
            steps {
                sh 'docker buildx build --platform linux/arm64 --network=host -t $DOCKER_IMAGE .'
            }
        }

        stage('Push to Docker Hub') {
            steps {
                sh '''
                echo "$DOCKER_CREDENTIALS" | docker login -u "$DOCKER_CREDENTIALS" --password-stdin

                docker push $DOCKER_IMAGE
                
                '''
            }
        }
    }
}
