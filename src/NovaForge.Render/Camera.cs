using System;
using OpenTK.Mathematics;

namespace NovaForge.Render
{
    public class Camera
    {
        public Vector3 Position { get; set; } = new Vector3(0, 0, 5);
        public float Yaw { get; set; } = -90f;
        public float Pitch { get; set; } = 0f;
        public float Fov { get; set; } = 70f;

        public Vector3 Front { get; private set; }
        public Vector3 Right { get; private set; }
        public Vector3 Up { get; private set; }

        public Camera()
        {
            UpdateVectors();
        }

        private void UpdateVectors()
        {
            float yawRad = MathHelper.DegreesToRadians(Yaw);
            float pitchRad = MathHelper.DegreesToRadians(Pitch);

            Front = Vector3.Normalize(new Vector3(
                (float)(Math.Cos(pitchRad) * Math.Cos(yawRad)),
                (float)Math.Sin(pitchRad),
                (float)(Math.Cos(pitchRad) * Math.Sin(yawRad))
            ));
            Right = Vector3.Normalize(Vector3.Cross(Front, Vector3.UnitY));
            Up = Vector3.Normalize(Vector3.Cross(Right, Front));
        }

        public Matrix4 GetViewMatrix()
        {
            UpdateVectors();
            return Matrix4.LookAt(Position, Position + Front, Up);
        }

        public Matrix4 GetProjectionMatrix(float aspectRatio)
        {
            return Matrix4.CreatePerspectiveFieldOfView(
                MathHelper.DegreesToRadians(Fov),
                aspectRatio,
                0.1f,
                1000f
            );
        }

        public void ProcessMouseMovement(float dx, float dy, float sensitivity = 0.1f)
        {
            Yaw += dx * sensitivity;
            Pitch -= dy * sensitivity;
            Pitch = Math.Clamp(Pitch, -89f, 89f);
            UpdateVectors();
        }

        public void MoveForward(float speed, float delta) { Position += Front * speed * delta; }
        public void MoveBack(float speed, float delta) { Position -= Front * speed * delta; }
        public void MoveLeft(float speed, float delta) { Position -= Right * speed * delta; }
        public void MoveRight(float speed, float delta) { Position += Right * speed * delta; }
        public void MoveUp(float speed, float delta) { Position += Up * speed * delta; }
        public void MoveDown(float speed, float delta) { Position -= Up * speed * delta; }
    }
}
