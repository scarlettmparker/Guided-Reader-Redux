import React, { useState, useEffect } from "react";
import { Card, CardBody, CardFooter, CardHeader } from "~/components/card";
import LikeButton from "~/components/like-button";
import ProfilePicture from "~/components/profile-picture";
import { unixToDate } from "~/utils/annotation";
import { AnnotationResponse } from "~/types";
import { marked } from "marked";
import styles from "./annotation.module.css";

interface AnnotationProps {
  annotation: AnnotationResponse;
}

const Annotation: React.FC<AnnotationProps> = (props) => {
  const { annotation } = props;
  const author = annotation.author;
  const [markedDescription, setMarkedDescription] = useState<string>("");

  useEffect(() => {
    const renderer = new marked.Renderer();

    // Add target="_blank" to all links to open in new tab
    renderer.link = function ({ href, title, text }) {
      const target = "_blank";
      const rel = "noopener noreferrer";
      return `<a href="${href}" title="${title || ""}" target="${target}" rel="${rel}">${text}</a>`;
    };

    marked.setOptions({
      breaks: true,
      renderer: renderer,
    });
  }, []);

  useEffect(() => {
    const fetchMarkedDescription = async () => {
      setMarkedDescription(await marked(annotation.description));
    };
    fetchMarkedDescription();
  }, [annotation.description]);

  return (
    <Card className={styles.annotation}>
      <a href={`/user/${author.discord_id}`} className={styles.annotation_link}>
        <CardHeader className={styles.annotation_header}>
          <ProfilePicture
            avatar={author.avatar}
            discord_id={author.discord_id}
            className={styles.profile_picture}
          />
          <span className={styles.username}>{author.username}</span>
          <span className={styles.annotation_date}>
            {unixToDate(annotation.created_at)}
          </span>
        </CardHeader>
      </a>
      <CardBody>
        <div
          className={styles.annotation_description}
          dangerouslySetInnerHTML={{ __html: markedDescription }}
        />
      </CardBody>
      <CardFooter className={styles.annotation_footer}>
        <LikeButton className={styles.like_button} />
        <span className={styles.likes_count}>
          {annotation.likes - annotation.dislikes}
        </span>
        <LikeButton reverse={true} />
      </CardFooter>
    </Card>
  );
};

export default Annotation;
